/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef Crypt_H_
#define Crypt_H_

class ByteBuffer;
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

	static ByteBuffer CRC32(const ByteBuffer &data);

	static ByteBuffer MD5(const ByteBuffer &data);

	static ByteBuffer MD5MACDigest(const ByteBuffer &key, const ByteBuffer &data);
	static bool MD5MACVerify(const ByteBuffer &key, const ByteBuffer &data, const ByteBuffer &digest);
	
	static const size_t MD5MACKeyLength;
	static const size_t MD5MACDigestLength;

	static ByteBuffer Base64Encode(const ByteBuffer &data, bool insert_line_breaks = true, int max_line_length = 72);
	static ByteBuffer Base64Decode(const ByteBuffer &data);

	void SetAESEncryptKey(const ByteBuffer &key);
	void SetAESDecryptKey(const ByteBuffer &key);
	ByteBuffer AESEncrypt(const ByteBuffer &data);
	ByteBuffer AESDecrypt(const ByteBuffer &data);

	void ZlibResetCompress(unsigned int level = 9);
	void ZlibResetDecompress();
	ByteBuffer ZlibCompress(const ByteBuffer &data);
	ByteBuffer ZlibDecompress(const ByteBuffer &data);

protected:
	CryptPrivate *m_priv;

private:
	DECLARE_NO_COPY_CLASS(Crypt)

};

#endif
