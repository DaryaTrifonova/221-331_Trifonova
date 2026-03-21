#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <QVector>
#include <QString>

struct Cred {
    QString url;
    QString secretB64;
};

bool decryptVaultFromFile(const QString& vaultPath,
                          const QString& pin,
                          QVector<Cred>& credsOut,
                          QString& errorText);

bool decryptSecretFromBase64(const QString& secretB64,
                             const QString& pin,
                             QString& loginOut,
                             QString& passwordOut,
                             QString& errorText);

void secureClearQString(QString& s);

#endif // CRYPTO_UTILS_H
