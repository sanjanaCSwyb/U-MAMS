#ifndef UMAMS_PASSWORD_HASH_H
#define UMAMS_PASSWORD_HASH_H
#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QString>

namespace PasswordHash {
constexpr int iterations = 600000;
inline bool equal(const QByteArray &a, const QByteArray &b)
{
    if (a.size() != b.size()) return false;
    unsigned int difference = 0;
    for (qsizetype i=0; i<a.size(); ++i)
        difference |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    return difference == 0;
}
inline QString encode(const QString &password)
{
    QByteArray salt(24, Qt::Uninitialized);
    for (auto &byte : salt) byte = static_cast<char>(QRandomGenerator::system()->generate() & 0xff);
    const QByteArray digest = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), salt, iterations, 32);
    return QString("pbkdf2_sha256$%1$%2$%3").arg(iterations).arg(QString::fromLatin1(salt.toBase64()), QString::fromLatin1(digest.toBase64()));
}
inline bool verify(const QString &password, const QString &encoded)
{
    if (password.size()>1024 || encoded.isEmpty()) return false;
    if (encoded.startsWith("pbkdf2_sha256$")) {
        const auto fields=encoded.split('$');
        if (fields.size()!=4) return false;
        bool valid=false;
        const int rounds=fields[1].toInt(&valid);
        if (!valid || rounds<10000 || rounds>2000000) return false;
        const QByteArray salt=QByteArray::fromBase64(fields[2].toLatin1());
        const QByteArray expected=QByteArray::fromBase64(fields[3].toLatin1());
        if (salt.size()<16 || expected.size()!=32) return false;
        return equal(QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256,password.toUtf8(),salt,rounds,32),expected);
    }
    // Existing installations are upgraded only after verification; new writes
    // always use a salted password hash.
    static const QRegularExpression sha256("^[a-fA-F0-9]{64}$");
    if (sha256.match(encoded).hasMatch())
        return equal(QCryptographicHash::hash(password.toUtf8(),QCryptographicHash::Sha256).toHex(),encoded.toLatin1().toLower());
    if (encoded.contains('$')) return false;
    return equal(password.toUtf8(),encoded.toUtf8());
}
inline bool needsUpgrade(const QString &encoded)
{
    return !encoded.startsWith(QString("pbkdf2_sha256$%1$").arg(iterations));
}
}
#endif
