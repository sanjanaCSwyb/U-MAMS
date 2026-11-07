-- Select the intended database explicitly before importing this file with a
-- MySQL administrator. Back up first. No existing account/profile is changed.
-- The application account needs only EXECUTE on umams_register_student, not
-- INSERT on users/students or access to the private registration guard table.
SET NAMES utf8mb4 COLLATE utf8mb4_0900_ai_ci;

-- Add the required registration field without rewriting legacy identity data.
-- Existing profiles retain the empty default until their phone is provided.
SET @umams_has_phone = (
    SELECT COUNT(*) FROM information_schema.columns
    WHERE table_schema=DATABASE() AND table_name='students' AND column_name='phone_number'
);
SET @umams_phone_migration = IF(@umams_has_phone=0,
    'ALTER TABLE students ADD COLUMN phone_number VARCHAR(32) NOT NULL DEFAULT ''''',
    'SELECT 1');
PREPARE umams_phone_statement FROM @umams_phone_migration;
EXECUTE umams_phone_statement;
DEALLOCATE PREPARE umams_phone_statement;

-- Read-only legacy audit: an email repeated in matching users/students rows for
-- the same account is expected. Only distinct account owners are conflicts.
-- Only the conflict count is reported, never email addresses or other identity
-- details. Conflicts are never merged/deleted; new calls cannot reuse any
-- normalized email already present in either table.
SELECT COUNT(*) AS conflicting_email_groups
FROM (
    SELECT normalized_email
    FROM (
        SELECT LOWER(TRIM(email)) AS normalized_email,CONCAT('user:',user_id) AS owner_key
        FROM users WHERE TRIM(email)<>''
        UNION ALL
        SELECT LOWER(TRIM(email)),IF(user_id IS NULL,CONCAT('student:',student_id),CONCAT('user:',user_id))
        FROM students WHERE TRIM(email)<>''
    ) AS account_emails
    GROUP BY normalized_email
    HAVING COUNT(DISTINCT owner_key)>1
) AS conflicting_emails;

CREATE TABLE IF NOT EXISTS student_registration_guard (
    guard_id tinyint unsigned NOT NULL,
    PRIMARY KEY (guard_id),
    CONSTRAINT chk_student_registration_guard CHECK (guard_id=1)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

INSERT INTO student_registration_guard(guard_id) VALUES(1)
ON DUPLICATE KEY UPDATE guard_id=1;

DROP PROCEDURE IF EXISTS umams_register_student;
DELIMITER $$
CREATE DEFINER=CURRENT_USER PROCEDURE umams_register_student(
    IN p_name TEXT,
    IN p_email TEXT,
    IN p_registration_number TEXT,
    IN p_department TEXT,
    IN p_phone_number TEXT,
    IN p_password_hash TEXT
)
SQL SECURITY DEFINER
MODIFIES SQL DATA
BEGIN
    DECLARE v_name TEXT;
    DECLARE v_email TEXT;
    DECLARE v_registration TEXT;
    DECLARE v_department TEXT;
    DECLARE v_phone TEXT;
    DECLARE v_entered_phone TEXT;
    DECLARE v_local TEXT;
    DECLARE v_salt TEXT;
    DECLARE v_digest TEXT;
    DECLARE v_guard tinyint unsigned DEFAULT NULL;
    DECLARE v_user_id int;
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        ROLLBACK;
        RESIGNAL;
    END;

    SET v_name=TRIM(p_name),v_email=LOWER(TRIM(p_email)),
        v_registration=TRIM(p_registration_number),v_department=TRIM(p_department);
    SET v_entered_phone=TRIM(p_phone_number),
        v_phone=REGEXP_REPLACE(TRIM(p_phone_number),'[() -]','');
    SET v_local=SUBSTRING_INDEX(v_email,'@',1);
    SET v_salt=SUBSTRING_INDEX(SUBSTRING_INDEX(p_password_hash,'$',3),'$',-1),
        v_digest=SUBSTRING_INDEX(p_password_hash,'$',-1);
    IF v_name IS NULL OR CHAR_LENGTH(v_name) NOT BETWEEN 1 AND 150
       OR v_department IS NULL OR CHAR_LENGTH(v_department) NOT BETWEEN 1 AND 150
       OR REGEXP_LIKE(CONCAT(v_name,v_department),'[[:cntrl:]]','c')
       OR v_registration IS NULL OR CHAR_LENGTH(v_registration) NOT BETWEEN 1 AND 20
       OR REGEXP_LIKE(v_registration,'[^A-Za-z0-9_/-]','c')
       OR v_email IS NULL OR CHAR_LENGTH(v_email) NOT BETWEEN 1 AND 150
       OR CHAR_LENGTH(v_local)>64 OR LEFT(v_local,1)='.' OR RIGHT(v_local,1)='.'
       OR INSTR(v_local,'..')>0 OR REGEXP_LIKE(v_email,'[[:cntrl:]]','c')
       OR REGEXP_LIKE(SUBSTRING_INDEX(v_email,'@',-1),'(^|[.])[a-z0-9-]{64}','c')
       OR NOT REGEXP_LIKE(v_email,'^[a-z0-9.!#$%&''*+/=?^_`{|}~-]+@[a-z0-9]([a-z0-9-]*[a-z0-9])?([.][a-z0-9]([a-z0-9-]*[a-z0-9])?)+$','c')
       OR v_entered_phone IS NULL OR CHAR_LENGTH(v_entered_phone) NOT BETWEEN 1 AND 32
       OR NOT REGEXP_LIKE(v_entered_phone,'^[+]?[0-9() -]+$','c')
       OR REGEXP_LIKE(v_entered_phone,'[[:cntrl:]]','c')
       OR NOT REGEXP_LIKE(v_phone,'^[+]?[0-9]{7,15}$','c')
       OR p_password_hash IS NULL
       OR NOT REGEXP_LIKE(p_password_hash,'^pbkdf2_sha256[$]600000[$][A-Za-z0-9+/]{32}[$][A-Za-z0-9+/]{43}=$','c')
       OR OCTET_LENGTH(FROM_BASE64(v_salt))<>24 OR OCTET_LENGTH(FROM_BASE64(v_digest))<>32
       OR BINARY TO_BASE64(FROM_BASE64(v_salt))<>BINARY v_salt
       OR BINARY TO_BASE64(FROM_BASE64(v_digest))<>BINARY v_digest
       OR REGEXP_LIKE(p_password_hash,'[[:cntrl:]]','c') THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='UMAMS_REG_INVALID_INPUT';
    END IF;

    START TRANSACTION;
    -- The first consistent account reads occur only after this shared mutex has
    -- been acquired. Concurrent registrations therefore observe the preceding
    -- committed account even under InnoDB REPEATABLE READ.
    SELECT guard_id INTO v_guard FROM student_registration_guard WHERE guard_id=1 FOR UPDATE;
    IF v_guard IS NULL THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='UMAMS_REG_UNAVAILABLE';
    END IF;
    IF EXISTS(SELECT 1 FROM students WHERE student_id=v_registration) THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='UMAMS_REG_ID_EXISTS';
    END IF;
    IF EXISTS(SELECT 1 FROM users WHERE username=v_registration) THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='UMAMS_REG_USERNAME_EXISTS';
    END IF;
    IF EXISTS(SELECT 1 FROM users WHERE LOWER(TRIM(email))=v_email)
       OR EXISTS(SELECT 1 FROM students WHERE LOWER(TRIM(email))=v_email) THEN
        SIGNAL SQLSTATE '45000' SET MESSAGE_TEXT='UMAMS_REG_EMAIL_EXISTS';
    END IF;

    INSERT INTO users(username,password_hash,role,email,full_name,is_active)
    VALUES(v_registration,p_password_hash,'STUDENT',v_email,v_name,1);
    SET v_user_id=LAST_INSERT_ID();
    INSERT INTO students(student_id,user_id,student_name,email,department,phone_number)
    VALUES(v_registration,v_user_id,v_name,v_email,v_department,v_phone);
    COMMIT;
END$$
DELIMITER ;

-- Grant separately to the actual configured account/host, for example:
-- GRANT EXECUTE ON PROCEDURE selected_database.umams_register_student
--   TO 'configured_app_user'@'localhost';
-- Do not grant general users/students INSERT to the application account.
-- The default DEFINER is the administrator importing this file. A dedicated
-- retained definer can instead be selected at deployment; it requires SELECT,
-- INSERT on users/students and SELECT,UPDATE on student_registration_guard.
-- All account-creation services must use this routine (or take its guard row in
-- the same transaction). Independent privileged maintenance must be serialized
-- with registration; it is intentionally not granted to the application user.
