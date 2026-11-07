# U-MAMS

University Medical Absence Management System is a Qt Widgets desktop application
for students and administrators at Wayamba University of Sri Lanka.

Students can register, submit a medical absence with its period and missed courses,
and follow its status. Each successful submission receives an automatic Medical ID.
Administrators can search applications, approve or decline pending requests, export
report summaries as CSV, and view approved medical periods by month and registration
prefix. The monthly view includes periods that overlap the selected month and
shows each application’s full dates.

## Build and run

Use CMake 3.19 or newer, Qt 6.5 or newer with Widgets, SQL, and Network, and MySQL 8.
The Windows build uses the Qt 6.11.2 MinGW 64-bit kit. The Qt `QMYSQL` driver and
compatible MySQL client libraries must be available at runtime.

1. Create a database and follow the [database setup guide](database/README.md)
   to install the schema, migrations, and student registration procedure.
2. Provision the administrator account and the restricted MySQL application
   account as described in the guide. The schema starts with no user accounts.
   Students can create their accounts through **Register** on Student Login.
3. Copy [the configuration example](database/database.ini.example) to
   `%LOCALAPPDATA%\Wayamba\U-MAMS\database.ini` and enter your local database
   settings there.
4. Open `CMakeLists.txt` in Qt Creator, select the MinGW 64-bit kit, and build
   and run `Absense_execute_GUI`. Use the CMake `UMAMS_MYSQL_ROOT` setting if
   the MySQL client libraries are installed in a different folder.

The database scripts define the schema and registration logic. They contain no
preloaded student, administrator, or medical application records. Keep database
exports and credential files outside the repository.
