#ifdef _MSC_VER
	#pragma warning ( disable : 4786 )
#endif

#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: Crypt.cpp,v 1.9 2003-02-22 08:00:31 jason Exp $)

#include "Crypt.h"

#ifdef _MSC_VER
	#pragma warning ( push, 1)
	#pragma warning ( disable : 4702 )
#endif

#include "crypto/cryptlib.h"
#include "crypto/aes.h"
#include "crypto/filters.h"
#include "crypto/hex.h"
#include "crypto/rsa.h"
#include "crypto/randpool.h"
#include "crypto/osrng.h"
#include "crypto/files.h"
#include "crypto/sha.h"
#include "crypto/crc.h"
#include "crypto/md5.h"
#include "crypto/md5mac.h"
#include "crypto/base64.h"

#ifdef _MSC_VER
	#pragma warning ( pop )
#endif

using namespace CryptoPP;

class CryptPrivate
{

public:
	CryptPrivate() : enc_keyset(false), dec_keyset(false)
	{
	}
	
public:
	AESEncryption enc;
	AESDecryption dec;
	bool enc_keyset;
	bool dec_keyset;

};

Crypt::Crypt()
{
	m_priv = new CryptPrivate;
}

Crypt::~Crypt()
{
	delete m_priv;
}

static RandomNumberGenerator& GetRNG()
{
	static AutoSeededRandomPool random_pool(true, 128);
	return random_pool;
}

ByteBuffer Crypt::Random(size_t len)
{
	ByteBuffer data(len);
	byte *ptr = data.Lock();
	try
	{
		GetRNG().GenerateBlock(ptr, len);
	}
	catch (...)
	{
		data.Unlock();
		throw;
	}
	data.Unlock();
	return data;
}

void Crypt::RSAGenerateKey(unsigned int key_length, ByteBuffer &public_key, ByteBuffer &private_key)
{

	ByteBuffer private_key_temp = ByteBuffer(key_length * 10);
	ArraySink private_sink(private_key_temp.Lock(), key_length * 10);
	RSAES_OAEP_SHA_Decryptor *priv = NULL;
	try
	{
		priv = new RSAES_OAEP_SHA_Decryptor(GetRNG(), key_length);
		priv->DEREncode(private_sink);
		private_sink.MessageEnd();
	}
	catch (...)
	{
		private_key_temp.Unlock();
		delete priv;
		throw;
	}
	private_key_temp.Unlock();
	wxASSERT(private_sink.TotalPutLength() <= private_key_temp.Length());
	private_key = ByteBuffer(private_key_temp.Lock(), private_sink.TotalPutLength());
	private_key_temp.Unlock();

	ByteBuffer public_key_temp = ByteBuffer(key_length * 10);
	ArraySink public_sink(public_key_temp.Lock(), key_length * 10);
	try
	{
		RSAES_OAEP_SHA_Encryptor pub(*priv);
		pub.DEREncode(public_sink);
		public_sink.MessageEnd();
	}
	catch (...)
	{
		public_key_temp.Unlock();
		delete priv;
		throw;
	}
	public_key_temp.Unlock();
	wxASSERT(public_sink.TotalPutLength() <= public_key_temp.Length());
	public_key = ByteBuffer(public_key_temp.Lock(), public_sink.TotalPutLength());
	public_key_temp.Unlock();

	delete priv;

}

ByteBuffer Crypt::RSAEncrypt(const ByteBuffer &public_key, const ByteBuffer &plain_text)
{

	StringSource public_source(public_key.Lock(), public_key.Length(), true);
	RSAES_OAEP_SHA_Encryptor pub(public_source);

	ByteBuffer buff(public_key.Length());

	ArraySink *sink = new ArraySink(buff.Lock(), buff.Length());
	size_t len;
	
	try
	{
		PK_EncryptorFilter *filter = new PK_EncryptorFilter(GetRNG(), pub, sink);
		StringSource src(plain_text.Lock(), plain_text.Length(), true, filter);
		wxASSERT(sink->TotalPutLength() <= buff.Length());
		len = sink->TotalPutLength();
	}
	catch (...)
	{
		plain_text.Unlock();
		buff.Unlock();
		public_key.Unlock();
		throw;
	}
	
	plain_text.Unlock();
	buff.Unlock();

	ByteBuffer result(buff.Lock(), len);
	buff.Unlock();

	public_key.Unlock();

	return result;

}

ByteBuffer Crypt::RSADecrypt(const ByteBuffer &private_key, const ByteBuffer &cither_text)
{

	StringSource private_source(private_key.Lock(), private_key.Length(), true);
	RSAES_OAEP_SHA_Decryptor priv(private_source);

	ByteBuffer buff(cither_text.Length());
	ArraySink *sink = new ArraySink(buff.Lock(), buff.Length());
	size_t len;

	try
	{
		PK_DecryptorFilter *filter = new PK_DecryptorFilter(priv, sink);
		StringSource src(cither_text.Lock(), cither_text.Length(), true, filter);
		len = sink->TotalPutLength();
		wxASSERT(len <= buff.Length());
	}
	catch (...)
	{
		cither_text.Unlock();
		buff.Unlock();
		buff.Unlock();
		throw;
	}

	cither_text.Unlock();
	buff.Unlock();

	ByteBuffer result(buff.Lock(), len);
	buff.Unlock();
	
	private_key.Unlock();

	return result;

}

void Crypt::SetAESEncryptKey(const ByteBuffer &key)
{
	wxASSERT(key.Length() == 32);
	try
	{
		m_priv->enc.SetKey(key.Lock(), key.Length());
	}
	catch (...)
	{
		key.Unlock();
		throw;
	}
	key.Unlock();
	m_priv->enc_keyset = true;
}

void Crypt::SetAESDecryptKey(const ByteBuffer &key)
{
	wxASSERT(key.Length() == 32);
	try
	{
		m_priv->dec.SetKey(key.Lock(), key.Length());
	}
	catch (...)
	{
		key.Unlock();
		throw;
	}
	key.Unlock();
	m_priv->dec_keyset = true;
}

ByteBuffer Crypt::AESEncrypt(const ByteBuffer &data)
{

	wxASSERT(m_priv->enc_keyset);

	ByteBuffer src(data);

	int x = src.Length() % m_priv->enc.BlockSize();
	if (x > 0)
	{
		x = m_priv->enc.BlockSize() - x;
		src += Random(x);
	}

	wxASSERT((src.Length() % m_priv->enc.BlockSize()) == 0);
	int num_blocks = src.Length() / m_priv->enc.BlockSize();

	ByteBuffer buff(src.Length());

	try
	{
		m_priv->enc.ProcessAndXorMultipleBlocks(src.Lock(), NULL, buff.Lock(), num_blocks);
	}
	catch (...)
	{
		src.Unlock();
		buff.Unlock();
		throw;
	}

	src.Unlock();
	buff.Unlock();

	return buff;

}

ByteBuffer Crypt::AESDecrypt(const ByteBuffer &data)
{

	wxASSERT(m_priv->dec_keyset);

	wxASSERT((data.Length() % m_priv->dec.BlockSize()) == 0);
	int num_blocks = data.Length() / m_priv->dec.BlockSize();

	ByteBuffer buff(data.Length());

	try
	{
		m_priv->dec.ProcessAndXorMultipleBlocks(data.Lock(), NULL, buff.Lock(), num_blocks);
	}
	catch (...)
	{
		data.Unlock();
		buff.Unlock();
		throw;
	}

	data.Unlock();
	buff.Unlock();

	return buff;

}

ByteBuffer Crypt::CRC32(const ByteBuffer &data)
{

	CryptoPP::CRC32 crc32;

	try
	{
		crc32.Update(data.Lock(), data.Length());
		data.Unlock();
	}
	catch (...)
	{
		data.Unlock();
		throw;
	}

	ByteBuffer output(crc32.DigestSize());
	try
	{
		crc32.Final(output.Lock());
		output.Unlock();
	}
	catch (...)
	{
		output.Unlock();
		throw;
	}

	return output;

}

ByteBuffer Crypt::MD5(const ByteBuffer &data)
{

	CryptoPP::MD5 md5;

	try
	{
		md5.Update(data.Lock(), data.Length());
		data.Unlock();
	}
	catch (...)
	{
		data.Unlock();
		throw;
	}

	ByteBuffer output(md5.DigestSize());
	try
	{
		md5.Final(output.Lock());
		output.Unlock();
	}
	catch (...)
	{
		output.Unlock();
		throw;
	}

	return output;

}

const size_t Crypt::MD5MACKeyLength = MD5MAC::KEYLENGTH;
const size_t Crypt::MD5MACDigestLength = MD5MAC::DIGESTSIZE;

ByteBuffer Crypt::MD5MACDigest(const ByteBuffer &key, const ByteBuffer &data)
{
	
	wxASSERT(key.Length() == MD5MACKeyLength);
	
	MD5MAC mac;

	try
	{
		mac.SetKey(key.Lock(), key.Length());
		key.Unlock();
	}
	catch (...)
	{
		key.Unlock();
		throw;
	}

	ByteBuffer digest(MD5MACDigestLength);

	try
	{
		mac.CalculateDigest(digest.Lock(), data.Lock(), data.Length());
	}
	catch (...)
	{
		digest.Unlock();
		data.Unlock();
		throw;
	}

	digest.Unlock();
	data.Unlock();
	return digest;

}

bool Crypt::MD5MACVerify(const ByteBuffer &key, const ByteBuffer &data, const ByteBuffer &digest)
{

	wxASSERT(key.Length() == MD5MACKeyLength);
	wxASSERT(digest.Length() == MD5MACDigestLength);

	MD5MAC mac;

	try
	{
		mac.SetKey(key.Lock(), key.Length());
		key.Unlock();
	}
	catch (...)
	{
		key.Unlock();
		throw;
	}

	bool match;
	try
	{
		match = mac.VerifyDigest(digest.Lock(), data.Lock(), data.Length());
	}
	catch (...)
	{
		digest.Unlock();
		data.Unlock();
		throw;
	}

	digest.Unlock();
	data.Unlock();
	return match;

}

ByteBuffer Crypt::Base64Encode(const ByteBuffer &data, bool insert_line_breaks, int max_line_length)
{

	ByteBuffer buff(data.Length() * 4);
	ArraySink *sink = new ArraySink(buff.Lock(), buff.Length());
	size_t len;

	Base64Encoder *filter = new Base64Encoder(sink, insert_line_breaks, max_line_length);
	StringSource src(data.Lock(), data.Length(), true, filter);
	len = sink->TotalPutLength();
	wxASSERT(len <= buff.Length());

	buff.Unlock();

	ByteBuffer result(buff.Lock(), len);
	buff.Unlock();
	
	data.Unlock();

	return result;

}

ByteBuffer Crypt::Base64Decode(const ByteBuffer &data)
{

	ByteBuffer buff(data.Length());
	ArraySink *sink = new ArraySink(buff.Lock(), buff.Length());
	size_t len;

	Base64Decoder *filter = new Base64Decoder(sink);
	StringSource src(data.Lock(), data.Length(), true, filter);
	len = sink->TotalPutLength();
	wxASSERT(len <= buff.Length());

	buff.Unlock();

	ByteBuffer result(buff.Lock(), len);
	buff.Unlock();
	
	data.Unlock();

	return result;

}
