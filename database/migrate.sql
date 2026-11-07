-- Run with a MySQL administrator after a backup, with the intended database selected.
-- Stop older application versions before this migration and deploy the updated
-- repository and counter-table privileges together. No stored Medical ID changes.

-- The nonunique index supports full UTC day-range reads during lazy seeding and
-- also keeps a student_id-leading index available before the old unique key drops.
SET @umams_has_day_index = (
    SELECT COUNT(*) FROM information_schema.statistics
    WHERE table_schema=DATABASE() AND table_name='medical_applications'
      AND index_name='idx_application_student_submitted'
);
SET @umams_migration = IF(@umams_has_day_index=0,
    'ALTER TABLE medical_applications ADD KEY idx_application_student_submitted (student_id,submitted_at)',
    'SELECT 1');
PREPARE umams_migration_statement FROM @umams_migration;
EXECUTE umams_migration_statement;
DEALLOCATE PREPARE umams_migration_statement;

-- A displayed student-MM-DD-number may recur in another calendar year. Record
-- identity is application_id, so the older student+Medical-ID unique key is invalid.
SET @umams_has_old_unique = (
    SELECT COUNT(*) FROM information_schema.statistics
    WHERE table_schema=DATABASE() AND table_name='medical_applications'
      AND index_name='uq_application_student_medical'
);
SET @umams_migration = IF(@umams_has_old_unique>0,
    'ALTER TABLE medical_applications DROP INDEX uq_application_student_medical',
    'SELECT 1');
PREPARE umams_migration_statement FROM @umams_migration;
EXECUTE umams_migration_statement;
DEALLOCATE PREPARE umams_migration_statement;

CREATE TABLE IF NOT EXISTS medical_submission_counters (
    student_id varchar(20) NOT NULL,
    submission_date date NOT NULL,
    last_number bigint unsigned NOT NULL DEFAULT 0,
    PRIMARY KEY (student_id,submission_date),
    CONSTRAINT fk_medical_submission_counter_student
        FOREIGN KEY (student_id) REFERENCES students(student_id)
        ON DELETE RESTRICT ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- No bulk renumbering or data seed is needed. The first successful submission
-- for a student/date seeds its locked counter from that full day's stored records.
-- Separately grant SELECT, INSERT and UPDATE on this new table to the existing
-- restricted application account (using its actual configured account and host).
