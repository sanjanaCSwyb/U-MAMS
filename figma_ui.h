#ifndef FIGMA_UI_H
#define FIGMA_UI_H

#include <QApplication>
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QDialog>
#include <QEvent>
#include <QFile>
#include <QFontDatabase>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTimer>
#include <QVector>
#include <QtMath>
#include "app_session.h"
#include "dashboard.h"
#include "studentdashboard.h"
#include "medicalapplication.h"
#include "dialog.h"
#include "approvedecline.h"
#include "notifications.h"
#include "reports.h"
#include "monthlyapprovedmedicals.h"
#include "studentprofile.h"
#include "myapplications.h"
#include "changepassword.h"

namespace FigmaUi {
// A parent's local stylesheet outranks the application stylesheet in Qt. Give
// message boxes their own canonical style so login-window backgrounds cannot
// replace the button fill while leaving the button text white.
class PopupAppearance final : public QObject {
    bool applying = false;
protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (applying || (event->type() != QEvent::Polish && event->type() != QEvent::Show))
            return QObject::eventFilter(watched, event);
        auto *box = qobject_cast<QMessageBox *>(watched);
        if (!box) return QObject::eventFilter(watched, event);
        applying = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
        box->setOption(QMessageBox::Option::DontUseNativeDialog, true);
#endif
        QString style = qApp->styleSheet();
        if (style.isEmpty()) {
            QFile file(":/umams.qss");
            if (file.open(QFile::ReadOnly | QFile::Text)) style = QString::fromUtf8(file.readAll());
        }
        if (!style.isEmpty() && box->styleSheet() != style) box->setStyleSheet(style);
        applying = false;
        return QObject::eventFilter(watched, event);
    }
public:
    explicit PopupAppearance(QObject *parent) : QObject(parent)
    {
        setObjectName("umamsPopupAppearance");
    }
};

inline void installApplicationPolicies()
{
    if (qApp && !qApp->findChild<QObject *>("umamsPopupAppearance"))
        qApp->installEventFilter(new PopupAppearance(qApp));
}

inline void configureWindowControls(QWidget *window)
{
    if (!window || !window->isWindow() || qobject_cast<QMessageBox *>(window)) return;
    auto flags = window->windowFlags();
    flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
             Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint;
    flags &= ~Qt::WindowContextHelpButtonHint;
    if (flags != window->windowFlags()) window->setWindowFlags(flags);
}

inline void loadFont()
{
    installApplicationPolicies();
    static bool loaded = false;
    if (loaded) return;
    loaded = true;
    QFontDatabase::addApplicationFont(":/assets/Inter.ttf");
    QFont font("Inter");
    font.setPixelSize(12);
    QApplication::setFont(font);
}

inline QString scaledStyle(const QString &style, qreal scale)
{
    static const QRegularExpression pixels(QStringLiteral("([0-9]+(?:\\.[0-9]+)?)px"));
    QString result;
    int previous = 0;
    auto matches = pixels.globalMatch(style);
    while (matches.hasNext()) {
        const auto match = matches.next();
        result += style.mid(previous, match.capturedStart() - previous);
        const qreal value = match.captured(1).toDouble();
        const QString prefix = style.left(match.capturedStart());
        const int declarationStart = qMax(prefix.lastIndexOf(';'), prefix.lastIndexOf('{')) + 1;
        const bool radius = prefix.mid(declarationStart).contains("radius", Qt::CaseInsensitive);
        // Rounding a pill's radius upward can exceed half its rounded height,
        // which makes Qt paint square corners at fractional window scales.
        const int length = radius ? qFloor(value * scale) : qRound(value * scale);
        result += QString::number(value == 0 ? 0 : qMax(1, length)) + "px";
        previous = match.capturedEnd();
    }
    return result + style.mid(previous);
}

inline QFont scaledFont(QFont font, qreal scale)
{
    if (font.pixelSize() > 0)
        font.setPixelSize(qMax(1, qRound(font.pixelSize() * scale)));
    else
        font.setPointSizeF(qMax(0.5, font.pointSizeF() * scale));
    return font;
}

inline bool studentContext(QWidget *root)
{
    Q_UNUSED(root);
    return AppSession::isStudent();
}

inline bool canNavigate(const QString &route)
{
    if (!AppSession::authenticated()) return false;
    if (route == "review" || route == "reports" || route == "admin-applications" || route == "monthly-medicals")
        return AppSession::isAdmin();
    if (route == "medical" || route == "profile" || route == "my-applications")
        return AppSession::isStudent();
    return route == "dashboard" || route == "logout" ||
           route == "applications" || route == "notifications" || route == "password";
}

inline void removeAdminSubmissionNavigation(QWidget *root)
{
    if (!AppSession::isAdmin()) return;
    for (auto *button : root->findChildren<QAbstractButton *>()) {
        if (button->property("figmaNav").toString() != "medical") continue;
        const QRect removed = button->geometry();
        QWidget *parent = button->parentWidget();
        const auto siblings = parent->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
        int previousY = -1;
        for (auto *sibling : siblings) {
            const QRect r = sibling->geometry();
            if (sibling != button && (qobject_cast<QAbstractButton *>(sibling) || qobject_cast<QLabel *>(sibling)) &&
                r.x() == removed.x() && r.width() == removed.width() && r.height() == removed.height() &&
                r.y() < removed.y()) previousY = qMax(previousY, r.y());
        }
        const int pitch = previousY >= 0 ? removed.y() - previousY : removed.height() + 13;
        button->hide();
        button->setEnabled(false);
        button->setProperty("figmaNav", QString());
        // Compact both button rows and the current-page label before Controller
        // captures their native geometry; Student sidebar positions stay intact.
        for (auto *sibling : siblings) {
            const QRect r = sibling->geometry();
            if (sibling != button && (qobject_cast<QAbstractButton *>(sibling) || qobject_cast<QLabel *>(sibling)) &&
                r.x() == removed.x() && r.width() == removed.width() && r.height() == removed.height() &&
                r.y() > removed.y()) sibling->move(r.x(), r.y() - pitch);
        }
    }
}

inline void addMonthlyMedicalsNavigation(QWidget *root)
{
    if (!AppSession::isAdmin()) return;
    auto *sidebar = root->findChild<QWidget *>("sidebar");
    // Some existing screens place their sidebar controls directly on the canvas.
    if (!sidebar) sidebar = root->findChild<QWidget *>("figmaCanvas");
    if (!sidebar || sidebar->findChild<QAbstractButton *>("btnSidebarMonthlyMedicals")) return;
    int lastBottom = -1;
    for (auto *entry : sidebar->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly)) {
        const QRect geometry = entry->geometry();
        if (!entry->isEnabled() || (!qobject_cast<QAbstractButton *>(entry) && !qobject_cast<QLabel *>(entry))) continue;
        if (geometry.x() == 16 && geometry.width() == 188 && geometry.height() >= 35 && geometry.height() <= 50)
            lastBottom = qMax(lastBottom, geometry.y() + geometry.height());
    }
    if (lastBottom < 0 || lastBottom + 55 > sidebar->height()) return;
    auto *button = new QPushButton("Monthly Medicals", sidebar);
    button->setObjectName("btnSidebarMonthlyMedicals");
    button->setProperty("figmaNav", "monthly-medicals");
    button->setGeometry(16, lastBottom + 13, 188, 42);
    button->setAutoDefault(false);
    button->setCursor(Qt::PointingHandCursor);
    button->setToolTip("Approved medical periods by month and registration number group");
    button->setStyleSheet("QPushButton#btnSidebarMonthlyMedicals {font-family:Inter;font-size:12px;color:white;background:transparent;border:none;border-radius:7px;text-align:left;padding-left:16px;} QPushButton#btnSidebarMonthlyMedicals:hover,QPushButton#btnSidebarMonthlyMedicals:focus {background:#922F48;}");
}

inline void configureRoleNavigation(QWidget *root)
{
    if (!AppSession::authenticated()) return;
    removeAdminSubmissionNavigation(root);
    addMonthlyMedicalsNavigation(root);
    for (auto *label : root->findChildren<QLabel *>()) {
        const QString name = label->objectName();
        if ((name.contains("Header", Qt::CaseInsensitive) || name.contains("Audience", Qt::CaseInsensitive)) &&
            (label->text() == "Admin" || label->text() == "User"))
            label->setText(AppSession::isStudent() ? "Student" : "Admin");
        if (AppSession::isStudent() && name == "navActiveMedical")
            label->setText("Medical Application");
    }
    if (!AppSession::isStudent()) return;
    for (auto *button : root->findChildren<QAbstractButton *>()) {
        const QString route = button->property("figmaNav").toString();
        if (route == "applications") {
            button->setText("My Applications");
            button->setProperty("figmaNav", "my-applications");
        } else if (route == "review") {
            button->setText("My Profile");
            button->setProperty("figmaNav", "profile");
        } else if (route == "reports") {
            button->setText("Change Password");
            button->setProperty("figmaNav", "password");
        } else if (route == "medical") {
            button->setText("Medical Application");
        }
        if (button->objectName().contains("Audience", Qt::CaseInsensitive)) {
            button->setText("Student");
            button->setToolTip("Change your password");
        }
    }
}

inline void navigate(QWidget *root, const QString &route)
{
    if (!canNavigate(route)) return;
    if (route == "dashboard" || route == "logout") {
        const bool student = studentContext(root);
        const QString target = student ? "StudentDashboard" : "Dashboard";
        QWidget *ancestor = root->parentWidget();
        while (ancestor && (route == "logout" || ancestor->metaObject()->className() != target))
            ancestor = ancestor->parentWidget();
        if (route == "logout") AppSession::logout();
        if (ancestor || route == "logout") {
            for (QWidget *p = root; p && p != ancestor;) {
                QWidget *parent = p->parentWidget();
                if (auto *dialog = qobject_cast<QDialog *>(p)) dialog->reject();
                p = parent;
            }
            return;
        }
        if (QString(root->metaObject()->className()) == target) return;
        if (student) { StudentDashboard page(root); page.exec(); }
        else { Dashboard page(root); page.exec(); }
    } else if (route == "medical") { MedicalApplication page(root); page.exec(); }
    else if (route == "applications" && AppSession::isStudent()) { MyApplications page(root); page.exec(); }
    else if (route == "applications" || route == "admin-applications") { Dialog page(root); page.exec(); }
    else if (route == "review") { ApproveDecline page(root); page.exec(); }
    else if (route == "notifications") { Notifications page(root); page.exec(); }
    else if (route == "reports") { Reports page(root); page.exec(); }
    else if (route == "monthly-medicals") { MonthlyApprovedMedicals page(root); page.exec(); }
    else if (route == "profile") { StudentProfile page(root); page.exec(); }
    else if (route == "my-applications") { MyApplications page(root); page.exec(); }
    else if (route == "password") { ChangePassword page(root); page.exec(); }
    for (QWidget *page=root; page; page=page->parentWidget())
        if (page->metaObject()->indexOfMethod("refreshData()") >= 0)
            QMetaObject::invokeMethod(page,"refreshData",Qt::DirectConnection);
}

class Controller final : public QObject {
    struct Element {
        QPointer<QWidget> widget;
        QRect geometry;
        QString style;
        QFont font;
        QString lastAppliedStyle;
    };
    struct Table {
        QPointer<QTableWidget> widget;
        QVector<int> widths;
        QVector<int> heights;
        int headerHeight;
        int defaultHeight;
        QFont horizontalHeaderFont;
        QFont verticalHeaderFont;
    };
    QPointer<QWidget> canvas;
    QPointer<QWidget> host;
    QVector<Element> elements;
    QVector<Table> tables;
    qreal scale = 1;
    bool applying = false;

    void applyTables()
    {
        for (const auto &entry : tables) {
            auto *table = entry.widget.data();
            if (!table) continue;
            for (int column = 0; column < entry.widths.size(); ++column)
                if (table->horizontalHeader()->sectionResizeMode(column) != QHeaderView::Stretch)
                    table->setColumnWidth(column, qRound(entry.widths[column] * scale));
            table->horizontalHeader()->setFixedHeight(qRound(entry.headerHeight * scale));
            table->verticalHeader()->setDefaultSectionSize(qRound(entry.defaultHeight * scale));
            table->horizontalHeader()->setFont(scaledFont(entry.horizontalHeaderFont, scale));
            table->verticalHeader()->setFont(scaledFont(entry.verticalHeaderFont, scale));
            for (int row = 0; row < table->rowCount(); ++row) {
                table->setRowHeight(row, qRound((row < entry.heights.size() ? entry.heights[row] : entry.defaultHeight) * scale));
                for (int column = 0; column < table->columnCount(); ++column) {
                    QWidget *cell = table->cellWidget(row, column);
                    if (!cell) continue;
                    QList<QWidget *> children = cell->findChildren<QWidget *>();
                    children.prepend(cell);
                    for (auto *child : children) {
                        const QString currentStyle = child->styleSheet();
                        if (!child->property("figmaOriginalStyle").isValid() ||
                            (child->property("figmaLastAppliedStyle").isValid() &&
                             currentStyle != child->property("figmaLastAppliedStyle").toString()))
                            child->setProperty("figmaOriginalStyle", child->styleSheet());
                        const QString nextStyle = scaledStyle(child->property("figmaOriginalStyle").toString(), scale);
                        child->setStyleSheet(nextStyle);
                        child->setProperty("figmaLastAppliedStyle", nextStyle);
                        // The view positions the outer cell. Its nested badge widgets
                        // retain their own local coordinates and need the same scale.
                        if (child != cell) {
                            if (!child->property("figmaOriginalGeometry").isValid())
                                child->setProperty("figmaOriginalGeometry", child->geometry());
                            const QRect r = child->property("figmaOriginalGeometry").toRect();
                            child->setGeometry(qRound(r.x() * scale), qRound(r.y() * scale),
                                               qRound(r.width() * scale), qRound(r.height() * scale));
                        }
                    }
                }
            }
        }
    }

    void apply()
    {
        if (!canvas || !host || applying) return;
        applying = true;
        scale = qMin(host->width() / 1200.0, host->height() / 800.0);
        const QSize size(qRound(1200 * scale), qRound(800 * scale));
        canvas->setGeometry((host->width()-size.width())/2, (host->height()-size.height())/2, size.width(), size.height());
        for (auto &entry : elements) {
            auto *widget = entry.widget.data();
            if (!widget) continue;
            const QRect &r = entry.geometry;
            widget->setGeometry(qRound(r.x()*scale), qRound(r.y()*scale), qRound(r.width()*scale), qRound(r.height()*scale));
            widget->setFont(scaledFont(entry.font, scale));
            if (widget->styleSheet() != entry.lastAppliedStyle)
                entry.style = widget->styleSheet();
            entry.lastAppliedStyle = scaledStyle(entry.style, scale);
            widget->setStyleSheet(entry.lastAppliedStyle);
            if (auto *label = qobject_cast<QLabel *>(widget)) {
                if (label->objectName().endsWith("logo", Qt::CaseInsensitive)) {
                    QPixmap crest(":/assets/wayamba_logo.png");
                    label->setPixmap(crest.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
            }
        }
        applyTables();
        applying = false;
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == host && (event->type() == QEvent::Resize || event->type() == QEvent::Show)) apply();
        return QObject::eventFilter(watched, event);
    }

public:
    explicit Controller(QWidget *root) : QObject(root)
    {
        setObjectName("figmaUiController");
        canvas = root->findChild<QWidget *>("figmaCanvas");
        if (!canvas) return;
        configureRoleNavigation(root);
        host = canvas->parentWidget();
        for (QWidget *widget : canvas->findChildren<QWidget *>()) {
            if (widget->objectName().isEmpty() || widget->objectName().startsWith("qt_")) continue;
            bool internal = false;
            for (QWidget *p = widget->parentWidget(); p && p != canvas; p = p->parentWidget())
                if (qobject_cast<QAbstractScrollArea *>(p)) { internal = true; break; }
            if (internal) continue;
            elements.push_back({widget, widget->geometry(), widget->styleSheet(), widget->font(), widget->styleSheet()});
            widget->setMinimumSize(0,0);
            widget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            if (auto *label = qobject_cast<QLabel *>(widget)) label->setAttribute(Qt::WA_TransparentForMouseEvents);
            if (auto *button = qobject_cast<QAbstractButton *>(widget)) {
                const QString route = button->property("figmaNav").toString();
                if (!route.isEmpty()) connect(button, &QAbstractButton::clicked, root, [root,route]{ navigate(root,route); });
            }
            if (auto *table = qobject_cast<QTableWidget *>(widget)) {
                Table entry{table, {}, {}, table->horizontalHeader()->height(), table->verticalHeader()->defaultSectionSize(),
                            table->horizontalHeader()->font(), table->verticalHeader()->font()};
                for (int column = 0; column < table->columnCount(); ++column) entry.widths.append(table->columnWidth(column));
                for (int row = 0; row < table->rowCount(); ++row) entry.heights.append(table->rowHeight(row));
                tables.append(entry);
                connect(table->model(), &QAbstractItemModel::rowsInserted, this, [this]{ QTimer::singleShot(0,this,[this]{ apply(); }); });
            }
        }
        host->installEventFilter(this);
        apply();
        QTimer::singleShot(0,this,[this]{ apply(); });
    }
};

inline void install(QWidget *root)
{
    loadFont();
    configureWindowControls(root);
    if (!root->findChild<QObject *>("figmaUiController")) new Controller(root);
}
}
#endif
