#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <QVector>
#include <QString>

struct Cred {
    QString url;
    QString login;
    QString password;
};

bool decryptVaultFromFile(const QString& vaultPath,
                          const QString& pin,
                          QVector<Cred>& credsOut,
                          QString& errorText);

#endif // CRYPTO_UTILS_H
