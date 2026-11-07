# U-MAMS MySQL database

The application uses MySQL 8 and Qt's QMYSQL driver. Existing users and student
profiles are retained. Applications, missed courses, decisions and notifications
are stored together in transactions. Students can only read and submit their own
applications; administrator decisions are accepted only while an application is
Pending.

Medical submission is available only to students. Administrators can view and
review submitted records. A successful submission assigns a Medical ID such as
`DEMO001-10-21-01`: Student ID, Sri Lankan submission month/day, and that student's
successful submission number for the full calendar date. The next submission by
that student on the same date receives `DEMO001-10-21-02`. Another student has an
independent sequence; the next Sri Lankan calendar day starts again at `01`.
Numbers use at least two digits, so a daily number above 99 is not truncated.

The repository captures `UTC_TIMESTAMP()` from MySQL once and converts it to
`Asia/Colombo` to select the counter date. The same captured instant is stored as
`submitted_at`. Medical From/To dates and the client computer clock do not select
the Medical ID date. A locked `medical_submission_counters` row keyed by
`(student_id, submission_date)` stores the complete date, including its year.
Concurrent submissions for the same student/date serialize on that row. The
counter increment, final Medical ID, application, courses and notifications commit
together; any failure rolls them all back and exposes no draft ID.

Existing Medical IDs remain unchanged. The first submission for a student/date
lazily seeds its zero counter to at least the number of existing applications in
that complete Sri Lankan day, or the largest numeric suffix of an existing literal
`student-MM-DD-` ID from that same day, whichever is greater. Old records from a
previous year do not advance this year's counter. Because the displayed format
omits the year, the same displayed Medical ID can occur in different years; use
`application_id` or the application code for unique record identity.

## Local configuration

The example uses `umams_db` on `127.0.0.1:3306`. Connection settings are
kept outside the source tree at:

`%LOCALAPPDATA%/Wayamba/U-MAMS/database.ini`

This file contains the restricted database account's password. Do not commit it
or share it. The application does not need the MySQL root account. Existing login
names and passwords are preserved; passwords are stored as salted PBKDF2-SHA256
hashes. New passwords must contain 8–128 characters.

Windows setup tools running from a packaged application can redirect AppData
writes into that package's `LocalCache/Local` folder, even when the requested
path looks correct. Verify the settings from the normal U-MAMS executable, and
verify the native file-handle path when diagnosing installation issues. Keep the active configuration in the real LocalAppData folder with access limited
to the user who runs U-MAMS.

An empty database shows zero counts and empty lists. The old GUI demonstration
applications are not inserted into the real database. Submit a medical application
as a student, then sign in as an administrator to review it.

## New installation

1. Back up any existing database.
2. Import `schema.sql` with a MySQL administrator account.
3. Stop older application versions, select the intended database and run
   `migrate.sql`. It creates the daily counter table and supporting date index,
   and removes the obsolete `uq_application_student_medical` unique index so
   display IDs can recur across different years. It does not alter old IDs,
   delete records, or prepopulate counter values. Run the migration before the
   updated executable is used.
4. Set up administrator accounts separately. For student self-registration,
   run `register_student.sql` after importing `schema.sql`, in the intended
   database with the dedicated locked definer described below.
   It does not modify existing users, student profiles, passwords or Medical IDs.
5. Create a dedicated MySQL account. Grant SELECT on `users` and `students`,
   UPDATE(`password_hash`) on `users`, SELECT/INSERT/UPDATE on
   `medical_applications`, `notifications`, and `medical_submission_counters`,
   and SELECT/INSERT on `missed_courses`. Existing application accounts also
   need SELECT/INSERT/UPDATE on the new counter table before deployment. No
   DROP, ALTER, user-management or DELETE privileges are required by the
   running application. Schema creation and migration use a separate MySQL
   administrator account. Student registration additionally requires EXECUTE
   on the single `umams_register_student` procedure; do not grant the application
   account INSERT on `users` or `students`.
6. Copy `database.ini.example` to the configuration location above and supply the
   restricted account's settings.
7. Build with the existing Qt 6.11 MinGW kit. CMake copies the installed MySQL
   client DLL, OpenSSL DLLs and QMYSQL plugin beside the executable. The cache
   setting `UMAMS_MYSQL_ROOT` selects the local MySQL installation.

Environment overrides are available for deployment and isolated tests:
`UMAMS_DB_CONFIG`, `UMAMS_DB_HOST`, `UMAMS_DB_PORT`, `UMAMS_DB_NAME`,
`UMAMS_DB_USER`, `UMAMS_DB_PASSWORD`. Never put secrets on a command line or
commit a local configuration file.

If login reports a database configuration problem, check the file path shown in
the message. The normal Qt configuration location is preferred; on Windows the
application also checks the documented `%LOCALAPPDATA%` location when that file
is absent. An explicit `UMAMS_DB_CONFIG` selects only that file. Missing,
unreadable or incomplete settings produce a configuration error instead of trying
a default account without a password. Remove empty `UMAMS_DB_*` overrides from
Qt Creator's Run Environment, especially `UMAMS_DB_PASSWORD`. An environment-only
setup must explicitly supply `UMAMS_DB_NAME`, `UMAMS_DB_USER` and
`UMAMS_DB_PASSWORD`; host and port default to `127.0.0.1` and `3306`.

Timestamps are stored/read in UTC and displayed in Asia/Colombo. Reports export
the actual records as UTF-8 CSV. No email messages are sent by this integration.

## Student self-registration

`Database::registerStudent(name,email,registrationNumber,department,phoneNumber,password)`
is available before sign-in. It does not authenticate the new account or change
session state; an already authenticated caller must sign out first. Registration
number becomes both `users.username` and `students.student_id`. The repository
trims the four identity fields, lowercases email, and preserves the registration
number's case. Names and departments accept Unicode text with 1–150 characters
and reject control characters. Registration numbers contain 1–20 ASCII letters,
digits, slashes, underscores or hyphens, including forms such as `2024/SCI/001`.
The email must be a conventional mailbox and dotted domain, at most 150
characters with a local part of at most 64. No university-specific domain is
imposed. Passwords contain 8–128 characters; confirmation belongs to the form.
Phone number is also required. Its trimmed input is at most 32 characters and
may contain ASCII digits, a leading `+`, spaces, parentheses or hyphens. Removing
the separators must leave 7–15 digits with an optional leading `+`. The stored
value is normalized, for example `+94 (77) 123-4567` becomes `+94771234567`.
Phone numbers are not unique account identifiers.

The application hashes the password with the existing salted PBKDF2-SHA256
implementation before sending it to MySQL. The procedure accepts only its exact
600000-round encoding with a 24-byte salt and 32-byte digest. Plaintext
passwords are not sent to the procedure or stored. Registration errors expose
friendly validation/duplicate/unavailable messages without returning SQL or hash
contents.

Back up the database and select it explicitly before importing
`register_student.sql`. Its initial read-only audit reports only
`conflicting_email_groups`, the number of normalized email groups belonging to
multiple account owners; no email addresses or other identity details are
printed. The expected duplicate between
a matching `users` and `students` row is treated as one owner. Legacy conflicts
are reported but neither merged nor deleted; new registration cannot reuse any
email already found in either table. Existing account IDs, names, email spelling,
passwords and student links remain untouched.

The same registration SQL idempotently adds
`students.phone_number VARCHAR(32) NOT NULL DEFAULT ''` when that column is
absent. Existing profiles receive the empty default; their identity fields
remain intact. When the column already exists, the migration leaves its
definition and all stored phone values unchanged. Apply this column migration
before granting a definer column-level INSERT access to `phone_number`.

Deploy the procedure under a dedicated, retained, locked MySQL definer account.
The SQL template contains `CREATE DEFINER=CURRENT_USER PROCEDURE`; the privileged
installer replaces only that anchored definer marker with the validated locked
account and host. Do not grant that account interactive login. Its required
privileges are SELECT and INSERT on `users` and `students`, SELECT and UPDATE on
`student_registration_guard`, and EXECUTE on its own procedure. The
installer may narrow the table grants to the columns referenced by the routine;
its student INSERT grant must include the new `phone_number` column. Existing
table-level SELECT/INSERT grants already cover it. No additional direct INSERT
grant is needed by the application account. The running
application account receives only EXECUTE on
`selected_database.umams_register_student`, in addition to its existing data
access. It receives no general account/profile INSERT privilege, no access to
the guard table, and no definer credentials. Install the routine and EXECUTE
grant before enabling the registration form. Reimporting the SQL replaces the
routine, so reapply its routine-level grants afterwards.

The procedure owns a standalone transaction. It locks the singleton InnoDB row
`student_registration_guard(guard_id=1)` before its first account reads, checks
registration ID, username and case-insensitive normalized email across both
tables, then inserts the fixed `STUDENT` user and matching profile. Both inserts
commit together; an error rolls them back and releases the lock. Concurrent
self-registration calls serialize on that row, preventing duplicate email races
even when different registration numbers are submitted simultaneously. Existing
student-ID/username unique keys provide additional protection. There is no role
parameter and no dynamic SQL in the routine.

This mutex is the service boundary for account creation, rather than a new
global email constraint that could invalidate legacy data. Every normal
account-creation service must call this procedure or acquire its guard row in
the same transaction before checking/inserting accounts. Independent privileged
maintenance writers must be coordinated with self-registration; they can bypass
the service if they deliberately write directly to the tables. The application
account cannot do so. Call the routine outside an existing transaction, and
retain its locked definer account for as long as the routine exists.

The security and concurrency model follows MySQL's
[stored-object access controls](https://dev.mysql.com/doc/refman/8.0/en/stored-objects-security.html)
and [InnoDB locking reads](https://dev.mysql.com/doc/refman/8.0/en/innodb-locking-reads.html).

Password implementation references: [Qt QPasswordDigestor](https://doc.qt.io/qt-6/qpassworddigestor.html)
and [OWASP password storage guidance](https://cheatsheetseries.owasp.org/cheatsheets/Password_Storage_Cheat_Sheet.html).
