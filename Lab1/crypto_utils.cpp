#include "crypto_utils.h"

#include <QFile>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCryptographicHash>

#include <openssl/evp.h>

static bool sha256KeyFromPin_Qt(const QString& pin, unsigned char key32[32])
{
    QByteArray key = QCryptographicHash::hash(pin.toUtf8(), QCryptographicHash::Sha256);
    if (key.size() != 32) return false;
    memcpy(key32, key.constData(), 32);
    return true;
}

static const unsigned char IV_FIXED[16] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x10,0x11,0x12,0x13,0x14,0x15
};

static bool decryptAes256Cbc_FixedIv(const QByteArray& ciphertext,
                                     const unsigned char key32[32],
                                     const unsigned char iv16[16],
                                     QByteArray& plainOut)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    bool ok = false;
    int len = 0, plainLen = 0;
    plainOut.resize(ciphertext.size() + 16);

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key32, iv16) != 1) goto cleanup;

    if (EVP_DecryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(plainOut.data()), &len,
                          reinterpret_cast<const unsigned char*>(ciphertext.constData()), ciphertext.size()) != 1) goto cleanup;
    plainLen = len;

    if (EVP_DecryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(plainOut.data()) + plainLen, &len) != 1) goto cleanup;
    plainLen += len;

    plainOut.resize(plainLen);
    ok = true;

cleanup:
    EVP_CIPHER_CTX_free(ctx);
    if (!ok) plainOut.clear();
    return ok;
}

static bool decryptFieldBase64Bytes(const QString& b64,
                                    const unsigned char key32[32],
                                    QByteArray& plainOut)
{
    const QByteArray blob = QByteArray::fromBase64(b64.toUtf8());
    if (blob.isEmpty()) return false;

    return decryptAes256Cbc_FixedIv(blob, key32, IV_FIXED, plainOut);
}

bool decryptVaultFromFile(const QString& vaultPath,
                          const QString& pin,
                          QVector<Cred>& credsOut,
                          QString& errorText)
{
    credsOut.clear();
    errorText.clear();

    QFile f(vaultPath);
    if (!f.open(QIODevice::ReadOnly)) {
        errorText = "Не удалось открыть vault.enc";
        return false;
    }

    const QByteArray encFile = f.readAll();
    f.close();

    unsigned char key[32];
    if (!sha256KeyFromPin_Qt(pin, key)) {
        errorText = "Ошибка получения ключа";
        return false;
    }

    QByteArray jsonBytes;
    if (!decryptAes256Cbc_FixedIv(encFile, key, IV_FIXED, jsonBytes)) {
        errorText = "Неверный пароль";
        return false;
    }

    QJsonParseError jerr{};
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &jerr);
    if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
        errorText = "Неверный пароль";
        return false;
    }

    const QJsonObject root = doc.object();
    const QJsonValue credsVal = root.value("creds");
    if (!credsVal.isArray()) {
        errorText = "Неверный формат JSON";
        return false;
    }

    const QJsonArray arr = credsVal.toArray();
    if (arr.size() < 10) {
    }

    for (const QJsonValue& v : arr) {
        if (!v.isObject()) continue;
        const QJsonObject o = v.toObject();

        Cred c;
        c.url = o.value("url").toString();
        if (c.url.isEmpty()) continue;

        const QString secretB64 = o.value("secret").toString();
        if (secretB64.isEmpty()) {
            errorText = "Неверный формат JSON";
            credsOut.clear();
            return false;
        }

        QByteArray secretJsonBytes;
        if (!decryptFieldBase64Bytes(secretB64, key, secretJsonBytes)) {
            errorText = "Неверный пароль";
            credsOut.clear();
            return false;
        }

        QJsonParseError serr{};
        const QJsonDocument sdoc = QJsonDocument::fromJson(secretJsonBytes, &serr);
        if (serr.error != QJsonParseError::NoError || !sdoc.isObject()) {
            errorText = "Неверный пароль";
            credsOut.clear();
            return false;
        }

        const QJsonObject so = sdoc.object();
        c.login = so.value("login").toString();
        c.password = so.value("password").toString();


        if (c.login.isEmpty() || c.password.isEmpty()) {
            errorText = "Неверный формат secret";
            credsOut.clear();
            return false;
        }

        credsOut.push_back(c);
    }

    if (credsOut.isEmpty()) {
        errorText = "Неверный пароль";
        return false;
    }

    return true;
}
