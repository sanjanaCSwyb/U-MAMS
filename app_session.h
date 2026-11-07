#ifndef APP_SESSION_H
#define APP_SESSION_H

#include <QDialog>
#include <QTimer>
#include <QString>
#include <QWidget>

namespace AppSession {
enum class Role { None, Student, Admin };

// A completed login and an explicit Back action have different destinations.
enum LoginResult { LoginBackToPortal = 2, LoginAuthenticated = 3 };

struct Identity {
    int userId = 0;
    Role role = Role::None;
    QString username, studentId, fullName, email, department;
};

struct State {
    Role role = Role::None;
    QString principal;
    Identity identity;
    bool logoutRequested = false;
};

inline State &state()
{
    static State current;
    return current;
}

inline Role currentRole() { return state().role; }
inline QString principal() { return state().principal; }
inline bool isStudent() { return currentRole() == Role::Student; }
inline bool isAdmin() { return currentRole() == Role::Admin; }
inline bool authenticated() { return currentRole() != Role::None; }
inline Identity identity() { return state().identity; }
inline bool logoutRequested() { return state().logoutRequested; }
inline void clear() { state() = {}; }
inline void logout()
{
    const bool requested = authenticated() || logoutRequested();
    clear();
    state().logoutRequested = requested;
}

// The portal owns the authenticated dashboard lifetime. Closing a window clears
// its session without being mistaken for an explicit Logout action.
class LoginSession final {
public:
    LoginSession(Role role, const QString &principal)
    {
        state() = {role, principal, {}};
    }
    explicit LoginSession(const Identity &identity)
    {
        state() = {identity.role, identity.role == Role::Student ? identity.studentId : identity.username, identity};
    }
    ~LoginSession() { clear(); }
    LoginSession(const LoginSession &) = delete;
    LoginSession &operator=(const LoginSession &) = delete;
};

inline bool deny(QWidget *page)
{
    page->setProperty("accessDenied", true);
    page->setEnabled(false);
    page->hide();
    QTimer::singleShot(0, page, [page] {
        if (auto *dialog = qobject_cast<QDialog *>(page)) dialog->reject();
        else page->close();
    });
    return false;
}

inline bool protect(QWidget *page, Role required)
{
    if (currentRole() != required || required == Role::None) return deny(page);
    page->setProperty("accessDenied", false);
    return true;
}

inline bool protectAuthenticated(QWidget *page)
{
    if (!authenticated()) return deny(page);
    page->setProperty("accessDenied", false);
    return true;
}
}
#endif
