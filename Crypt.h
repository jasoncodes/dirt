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
	static ByteBuffer RSAEncrypt(ByteBuffer &public_key, ByteBuffer &plain_text);
	static ByteBuffer RSADecrypt(ByteBuffer &private_key, ByteBuffer &cither_text);
	
	void SetAESEncryptKey(const ByteBuffer &key);
	void SetAESDecryptKey(const ByteBuffer &key);
	ByteBuffer AESEncrypt(const ByteBuffer &data);
	ByteBuffer AESDecrypt(const ByteBuffer &data);

	static ByteBuffer MD5(const ByteBuffer &data);

protected:
	CryptPrivate *m_priv;

};

#endif
