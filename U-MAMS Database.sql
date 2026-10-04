CREATE DATABASE IF NOT EXISTS umams_db;

USE umams_db;
-- ============================================================
-- U-MAMS DATABASE
-- University Medical Absence Management System
-- Department: Material and Nano Science Department
-- ============================================================

CREATE DATABASE IF NOT EXISTS umams_db;

USE umams_db;


-- ============================================================
-- 1. USERS TABLE
-- Stores system login accounts
-- Roles: STUDENT / ADMIN
-- ============================================================

CREATE TABLE IF NOT EXISTS users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,

    username VARCHAR(50) NOT NULL UNIQUE,

    password_hash VARCHAR(255) NOT NULL,

    role ENUM('STUDENT', 'ADMIN') NOT NULL,

    email VARCHAR(150) NOT NULL,

    full_name VARCHAR(150) NOT NULL,

    is_active BOOLEAN NOT NULL DEFAULT TRUE,

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);


-- ============================================================
-- 2. STUDENTS TABLE
-- Stores student-specific information
-- ============================================================

CREATE TABLE IF NOT EXISTS students (
    student_id VARCHAR(20) PRIMARY KEY,

    user_id INT UNIQUE,

    student_name VARCHAR(150) NOT NULL,

    email VARCHAR(150) NOT NULL,

    department VARCHAR(150) NOT NULL
        DEFAULT 'Material and Nano Science Department',

    FOREIGN KEY (user_id)
        REFERENCES users(user_id)
        ON DELETE SET NULL
        ON UPDATE CASCADE
);


-- ============================================================
-- 3. MEDICAL APPLICATIONS TABLE
-- Main medical absence application
-- ============================================================

CREATE TABLE IF NOT EXISTS medical_applications (

    application_id INT AUTO_INCREMENT PRIMARY KEY,

    student_id VARCHAR(20) NOT NULL,

    medical_id VARCHAR(100) NOT NULL,

    reason_of_absence TEXT NOT NULL,

    medical_from DATE NOT NULL,

    medical_to DATE NOT NULL,

    status ENUM(
        'PENDING',
        'APPROVED',
        'DECLINED'
    ) NOT NULL DEFAULT 'PENDING',

    reviewed_by INT NULL,

    reviewed_at DATETIME NULL,

    submitted_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        ON UPDATE CURRENT_TIMESTAMP,

    FOREIGN KEY (student_id)
        REFERENCES students(student_id)
        ON DELETE RESTRICT
        ON UPDATE CASCADE,

    FOREIGN KEY (reviewed_by)
        REFERENCES users(user_id)
        ON DELETE SET NULL
        ON UPDATE CASCADE,

    CONSTRAINT chk_medical_period
        CHECK (medical_from <= medical_to)
);


-- ============================================================
-- 4. MISSED COURSES TABLE
-- Stores multiple courses belonging to one application
-- ============================================================

CREATE TABLE IF NOT EXISTS missed_courses (

    course_id INT AUTO_INCREMENT PRIMARY KEY,

    application_id INT NOT NULL,

    course_name VARCHAR(150) NOT NULL,

    course_code VARCHAR(50) NOT NULL,

    FOREIGN KEY (application_id)
        REFERENCES medical_applications(application_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- ============================================================
-- 5. NOTIFICATIONS TABLE
-- Stores notifications for students/admins
-- ============================================================

CREATE TABLE IF NOT EXISTS notifications (

    notification_id INT AUTO_INCREMENT PRIMARY KEY,

    user_id INT NOT NULL,

    application_id INT NULL,

    title VARCHAR(200) NOT NULL,

    message TEXT NOT NULL,

    notification_type ENUM(
        'SUBMISSION',
        'APPROVAL',
        'DECLINE',
        'GENERAL'
    ) NOT NULL DEFAULT 'GENERAL',

    is_read BOOLEAN NOT NULL DEFAULT FALSE,

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (user_id)
        REFERENCES users(user_id)
        ON DELETE CASCADE
        ON UPDATE CASCADE,

    FOREIGN KEY (application_id)
        REFERENCES medical_applications(application_id)
        ON DELETE SET NULL
        ON UPDATE CASCADE
);


-- ============================================================
-- 6. SYSTEM SETTINGS TABLE
-- Stores system-wide settings
-- ============================================================

CREATE TABLE IF NOT EXISTS system_settings (

    setting_id INT AUTO_INCREMENT PRIMARY KEY,

    setting_name VARCHAR(100) NOT NULL UNIQUE,

    setting_value VARCHAR(255) NOT NULL,

    description VARCHAR(255)
);


-- ============================================================
-- 7. EMAIL LOG TABLE
-- Stores email sending history
-- ============================================================

CREATE TABLE IF NOT EXISTS email_logs (

    email_id INT AUTO_INCREMENT PRIMARY KEY,

    application_id INT NULL,

    recipient_email VARCHAR(150) NOT NULL,

    cc_email VARCHAR(150) NULL,

    subject VARCHAR(255) NOT NULL,

    email_status ENUM(
        'PENDING',
        'SENT',
        'FAILED'
    ) NOT NULL DEFAULT 'PENDING',

    sent_at DATETIME NULL,

    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    FOREIGN KEY (application_id)
        REFERENCES medical_applications(application_id)
        ON DELETE SET NULL
        ON UPDATE CASCADE
);


-- ============================================================
-- INSERT DEFAULT ADMIN USER
-- Prototype login account
-- ============================================================

INSERT INTO users
(
    username,
    password_hash,
    role,
    email,
    full_name
)
SELECT
    'admin',
    '1234',
    'ADMIN',
    'admin@university.edu',
    'Department Administrator'
WHERE NOT EXISTS
(
    SELECT 1
    FROM users
    WHERE username = 'admin'
);


-- ============================================================
-- INSERT SAMPLE STUDENT USER
-- Prototype / testing account
-- ============================================================

INSERT INTO users
(
    username,
    password_hash,
    role,
    email,
    full_name
)
SELECT
    '249188',
    '1234',
    'STUDENT',
    '249188@student.university.edu',
    'Kasun Kaushal'
WHERE NOT EXISTS
(
    SELECT 1
    FROM users
    WHERE username = '249188'
);


-- ============================================================
-- INSERT SAMPLE STUDENT
-- ============================================================

INSERT INTO students
(
    student_id,
    user_id,
    student_name,
    email,
    department
)
SELECT
    '249188',
    user_id,
    'Kasun Kaushal',
    '249188@student.university.edu',
    'Material and Nano Science Department'
FROM users
WHERE username = '249188'
AND NOT EXISTS
(
    SELECT 1
    FROM students
    WHERE student_id = '249188'
);


-- ============================================================
-- ASSISTANT REGISTRAR EMAIL
-- ============================================================

INSERT INTO system_settings
(
    setting_name,
    setting_value,
    description
)
SELECT
    'assistant_registrar_email',
    'assistantregistrar@university.edu',
    'Email address of the Assistant Registrar who receives medical applications'
WHERE NOT EXISTS
(
    SELECT 1
    FROM system_settings
    WHERE setting_name = 'assistant_registrar_email'
);


-- ============================================================
-- DEPARTMENT NAME
-- ============================================================

INSERT INTO system_settings
(
    setting_name,
    setting_value,
    description
)
SELECT
    'department_name',
    'Material and Nano Science Department',
    'Department used by the U-MAMS system'
WHERE NOT EXISTS
(
    SELECT 1
    FROM system_settings
    WHERE setting_name = 'department_name'
);


-- ============================================================
-- CHECK DATABASE STRUCTURE
-- ============================================================

SHOW TABLES;