#ifndef Crypt_H_
#define Crypt_H_

#include "ByteBuffer.h"

class CryptPrivate;

class Crypt
{

public:
	Crypt();
	virtual ~Crypt();

	static ByteBuffer Random(size_t len);

	static void RSAGenerateKey(unsigned int key_length, ByteBuffer &public_key, ByteBuffer &private_key);
	static ByteBuffer RSAEncrypt(const ByteBuffer &public_key, const ByteBuffer &plain_text);
	static ByteBuffer RSADecrypt(const ByteBuffer &private_key, const ByteBuffer &cither_text);

	static ByteBuffer MD5(const ByteBuffer &data);

	static ByteBuffer MD5MACDigest(const ByteBuffer &key, const ByteBuffer &data);
	static bool MD5MACVerify(const ByteBuffer &key, const ByteBuffer &data, const ByteBuffer &digest);
	
	static const size_t MD5MACKeyLength;
	static const size_t MD5MACDigestLength;

	void SetAESEncryptKey(const ByteBuffer &key);
	void SetAESDecryptKey(const ByteBuffer &key);
	ByteBuffer AESEncrypt(const ByteBuffer &data);
	ByteBuffer AESDecrypt(const ByteBuffer &data);

protected:
	CryptPrivate *m_priv;

};

#endif
