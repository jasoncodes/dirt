package au.com.gslabs.dirt.lib.crypt.generic;

import au.com.gslabs.dirt.lib.util.ByteBuffer;

/*
This class is based on MD5.java from GNU Classpath which has been licensed
under the GPL. Original license follows:
*/

/* MD5.java -- Class implementing the MD5 algorithm as specified in RFC1321.
	 Copyright (C) 1999, 2002 Free Software Foundation, Inc.

Originally package gnu.java.security.provider, extracted and moved
into com.graeff.pwencode by Tom Aeby for practical reasons.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.
 
GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


public final class MD5 implements Cloneable
{
	
	private final int W[] = new int[16];
	private long bytecount;
	private int A;
	private int B;
	private int C;
	private int D;
	private final int key[] = new int[12];
	private final boolean modeMAC;
	
	/* constants specific for MD5MAC */
	private static final byte T0[] = new byte[] {
		(byte)0x97,(byte)0xef,(byte)0x45,(byte)0xac,(byte)0x29,(byte)0x0f,(byte)0x43,(byte)0xcd,(byte)0x45,
		(byte)0x7e,(byte)0x1b,(byte)0x55,(byte)0x1c,(byte)0x80,(byte)0x11,(byte)0x34 };
	private static final byte T1[] = new byte[] {
		(byte)0xb1,(byte)0x77,(byte)0xce,(byte)0x96,(byte)0x2e,(byte)0x72,(byte)0x8e,(byte)0x7c,(byte)0x5f,
		(byte)0x5a,(byte)0xab,(byte)0x0a,(byte)0x36,(byte)0x43,(byte)0xbe,(byte)0x18 };
	private static final byte T2[] = new byte[] {
		(byte)0x9d,(byte)0x21,(byte)0xb4,(byte)0x21,(byte)0xbc,(byte)0x87,(byte)0xb9,(byte)0x4d,(byte)0xa2,
		(byte)0x9d,(byte)0x27,(byte)0xbd,(byte)0xc7,(byte)0x5b,(byte)0xd7,(byte)0xc3 };

	public MD5(boolean modeMAC)
	{
		this.modeMAC = modeMAC;
		if (!this.modeMAC)
		{
			setKey(null);
			engineReset();
		}
	}

	public Object clone()
	{
		return new MD5 (this);
	}

	private MD5 (MD5 other)
	{
		this (other.modeMAC);
		this.bytecount = other.bytecount;
		this.A = other.A;
		this.B = other.B;
		this.C = other.C;
		this.D = other.D;
		System.arraycopy(other.key, 0, this.key, 0, other.key.length);
		System.arraycopy(other.W, 0, this.W, 0, other.W.length);
	}

	// Intialize the A,B,C,D needed for the hash
	public void engineReset()
	{
		bytecount = 0;
		A = key[0];
		B = key[1];
		C = key[2];
		D = key[3];
		for(int i = 0; i < 16; i++)
			W[i] = 0;
	}
	
	private static void copyBytes(int[] dest, int destOffsetBytes, byte[] src, int srcOffsetBytes, int byteCount)
	{
		if ((byteCount % 4) != 0)
		{
			throw new IllegalArgumentException("Must be multiple of 4 bytes");
		}
		for (int idxInt = 0; idxInt < byteCount/4; ++idxInt)
		{
			int srcBase = srcOffsetBytes + (idxInt*4);
			dest[destOffsetBytes/4 + idxInt] =
				((src[srcBase+0]&0xff) << 24) +
				((src[srcBase+1]&0xff) << 16) +
				((src[srcBase+2]&0xff) << 8) +
				((src[srcBase+3]&0xff) << 0);
		}
	}
	
	public void setKey(byte[] keyBytes)
	{
		if (keyBytes != null)
		{
			if (keyBytes.length != 16)
			{
				throw new IllegalArgumentException("Illegal key length");
			}
			
			int[] theKey = new int[12];
			
			/* The key setup requires MD5 block operations (without padding
			      or appended length.
			      MD5 can be obtained from MD5MAC by making K0 equal to the
			      MD5 initialization constants and K1 and K2 equal to 0.
			   */
			setKey(null);
			W[0] = key[0];
			W[1] = key[1];
			W[2] = key[2];
			W[3] = key[3];
			for (int i = 4; i < 12; ++i)
			{
				W[i] = 0;
			}
			
			/* derivation of K0 */
			A = key[0];
			B = key[1];
			C = key[2];
			D = key[3];
			
			copyBytes(W, 0, keyBytes, 0, 16);
			copyBytes(W, 16, T0, 0, 16);
			copyBytes(W, 32, T1, 0, 16);
			copyBytes(W, 48, T2, 0, 16);
			
			bytecount = 64;
			munch();
			
			copyBytes(W, 0, T0, 0, 16);
			copyBytes(W, 16, T1, 0, 16);
			copyBytes(W, 32, T2, 0, 16);
			copyBytes(W, 48, keyBytes, 0, 16);
			
			bytecount = 64;
			munch();
			
			theKey[0] = A;
			theKey[1] = B;
			theKey[2] = C;
			theKey[3] = D;
			
			/* derivation of K1 */
			A = key[0];
			B = key[1];
			C = key[2];
			D = key[3];
			
			copyBytes(W, 0, keyBytes, 0, 16);
			copyBytes(W, 16, T1, 0, 16);
			copyBytes(W, 32, T2, 0, 16);
			copyBytes(W, 48, T0, 0, 16);
			
			bytecount = 64;
			munch();
			
			copyBytes(W, 0, T1, 0, 16);
			copyBytes(W, 16, T2, 0, 16);
			copyBytes(W, 32, T0, 0, 16);
			copyBytes(W, 48, keyBytes, 0, 16);
			
			bytecount = 64;
			munch();
			
			theKey[4] = A;
			theKey[5] = B;
			theKey[6] = C;
			theKey[7] = D;
			
			/* derivation of K2 */
			A = key[0];
			B = key[1];
			C = key[2];
			D = key[3];
			
			copyBytes(W, 0, keyBytes, 0, 16);
			copyBytes(W, 16, T2, 0, 16);
			copyBytes(W, 32, T0, 0, 16);
			copyBytes(W, 48, T1, 0, 16);
			
			bytecount = 64;
			munch();
			
			copyBytes(W, 0, T2, 0, 16);
			copyBytes(W, 16, T0, 0, 16);
			copyBytes(W, 32, T1, 0, 16);
			copyBytes(W, 48, keyBytes, 0, 16);
			
			bytecount = 64;
			munch();
			
			theKey[8] = A;
			theKey[9] = B;
			theKey[10] = C;
			theKey[11] = D;
			
			System.arraycopy(theKey, 0, key, 0, 12);
			
			engineReset();
			
		}
		else
		{
			key[0] = 0x67452301;
			key[1] = 0xefcdab89;
			key[2] = 0x98badcfe;
			key[3] = 0x10325476;
			for (int i = 4; i < 12; ++i)
			{
				key[i] = 0;
			}
		}
	}

	public void engineUpdate (byte b)
	{
		int i = (int)bytecount % 64;
		int shift = (3 - i % 4) * 8;
		int idx = i / 4;

		// if you could index ints, this would be: W[idx][shift/8] = b
		W[idx] = (W[idx] & ~(0xff << shift)) | ((b & 0xff) << shift);

		// if we've filled up a block, then process it
		if ((++ bytecount) % 64 == 0)
			munch ();
	}

	public void engineUpdate (byte bytes[], int off, int len)
	{
		if (len < 0)
			throw new ArrayIndexOutOfBoundsException ();

		int end = off + len;
		while (off < end)
			engineUpdate (bytes[off++]);
	}

	public byte[] engineDigest()
	{
		long bitcount = bytecount * 8;
		engineUpdate ((byte)0x80); // 10000000 in binary; the start of the padding

		// add the rest of the padding to fill this block out, but leave 8
		// bytes to put in the original bytecount
		while ((int)bytecount % 64 != 56)
			engineUpdate ((byte)0);

		// add the length of the original, unpadded block to the end of
		// the padding
		W[14] = SWAP((int)(0xffffffff & bitcount));
		W[15] = SWAP((int)(0xffffffff & (bitcount >>> 32)));
		bytecount += 8;

	
		// digest the fully padded block
		munch ();

		if (modeMAC)
		{
			
			/* Assemble final block which is key dependent in MD5MAC */
			/* Build up in the buffer: a complete buffer will be transformed */
			for (int i = 0; i < 16; ++i)
			{
				W[i] = 0;
			}
			copyBytes(W, 16, T0, 0, 16);
			copyBytes(W, 32, T1, 0, 16);
			copyBytes(W, 48, T2, 0, 16);
			
			byte[] key2 = encode(new int[] { key[8], key[9], key[10], key[11] });
			
			for (int i = 0; i < 16; ++i)
			{
				int baseKey2 = (i*4) % 16;
				W[i] ^=
					((key2[baseKey2+3]&0xff) << 0) +
					((key2[baseKey2+2]&0xff) << 8) +
					((key2[baseKey2+1]&0xff) << 16) +
					((key2[baseKey2+0]&0xff) << 24);
			}
			
			bytecount = 64;
			munch();
			
		}
		
		byte[] result = encode(new int[] { A,B,C,D });
		engineReset ();
		return result;
	}
	
	private static byte[] encode(int[] input)
	{
		byte[] bytes = new byte[input.length*4];
		for (int idxInt = 0; idxInt < input.length; ++idxInt)
		{
			bytes[(idxInt*4)+3] = (byte)( (input[idxInt] >>> 24) & 0xff );
			bytes[(idxInt*4)+2] = (byte)( (input[idxInt] >>> 16) & 0xff );
			bytes[(idxInt*4)+1] = (byte)( (input[idxInt] >>> 8) & 0xff );
			bytes[(idxInt*4)+0] = (byte)( (input[idxInt] >>> 0) & 0xff );
		}
		return bytes;
	}

	private int F( int X, int Y, int Z)
	{
		return ((X & Y) | (~X & Z));
	}

	private int G( int X, int Y, int Z)
	{
		return ((X & Z) | (Y & ~Z));
	}

	private int H( int X, int Y, int Z)
	{
		return (X ^ Y ^ Z);
	}

	private int I( int X, int Y, int Z)
	{
		return (Y ^ (X | ~Z));
	}

	private int rotateLeft( int i, int count)
	{
		//Taken from FIPS 180-1
		return ( (i << count) | (i >>> (32 - count)) ) ;
	}

	/* Round 1. */
	private int FF( int a, int b, int c, int d, int k, int s, int i, int k2)
	{
		/* Let [abcd k s i] denote the operation */
		a += F(b,c,d) + k + i + k2;
		return b + rotateLeft(a, s); 
	} 
	/* Round 2. */
	private int GG( int a, int b, int c, int d, int k, int s, int i, int k2)
	{
		/* Let [abcd k s i] denote the operation */
		a += G(b,c,d) + k + i + k2;
		return b + rotateLeft(a, s);
	} 
	/* Round 3. */
	private int HH( int a, int b, int c, int d, int k, int s, int i, int k2)
	{
		/* Let [abcd k s t] denote the operation */
		a += H(b,c,d) + k + i + k2;
		return b + rotateLeft(a, s);
	} 

	/* Round 4. */
	private int II( int a, int b, int c, int d, int k, int s, int i, int k2)
	{
		/* Let [abcd k s t] denote the operation */
		a += I(b,c,d) + k + i + k2;
		return b + rotateLeft(a, s);
	} 

	private int SWAP(int n)
	{
		//Copied from md5.c in FSF Gnu Privacy Guard 0.9.2
		return (( (0xff & n) << 24) | ((n & 0xff00) << 8) | ((n >>> 8) & 0xff00) | (n >>> 24));
	}

	private void munch()
	{
		int AA,BB,CC,DD, j;
		int X[] = new int[16];

		/* Copy block i into X. */
		for(j = 0; j < 16; j++)
			X[j] = SWAP(W[j]);

		/* Save A as AA, B as BB, C as CC, and D as DD. */
		AA = A;
		BB = B;
		CC = C;
		DD = D;

		/* The hex constants are from md5.c 
		   in FSF Gnu Privacy Guard 0.9.2 */
		/* Round 1. */
		/* Let [abcd k s i] denote the operation
		   a = b + ((a + F(b,c,d) + X[k] + T[i]) <<< s). */
		/* Do the following 16 operations. */
		A = FF(A,B,C,D,  X[0],  7,  0xd76aa478, key[4]);
		D = FF(D,A,B,C,  X[1], 12,  0xe8c7b756, key[4]);
		C = FF(C,D,A,B,  X[2], 17,  0x242070db, key[4]);
		B = FF(B,C,D,A,  X[3], 22,  0xc1bdceee, key[4]);

		A = FF(A,B,C,D,  X[4],  7,  0xf57c0faf, key[4]);
		D = FF(D,A,B,C,  X[5], 12,  0x4787c62a, key[4]);
		C = FF(C,D,A,B,  X[6], 17,  0xa8304613, key[4]);
		B = FF(B,C,D,A,  X[7], 22,  0xfd469501, key[4]);

		A = FF(A,B,C,D,  X[8],  7,  0x698098d8, key[4]);
		D = FF(D,A,B,C,  X[9], 12,  0x8b44f7af, key[4]);
		C = FF(C,D,A,B, X[10], 17,  0xffff5bb1, key[4]);
		B = FF(B,C,D,A, X[11], 22,  0x895cd7be, key[4]);

		A = FF(A,B,C,D, X[12],  7,  0x6b901122, key[4]);
		D = FF(D,A,B,C, X[13], 12,  0xfd987193, key[4]);
		C = FF(C,D,A,B, X[14], 17,  0xa679438e, key[4]);
		B = FF(B,C,D,A, X[15], 22,  0x49b40821, key[4]);

		/* Round 2. */
		/* Let [abcd k s i] denote the operation
		   a = b + ((a + G(b,c,d) + X[k] + T[i]) <<< s). */
		/* Do the following 16 operations. */
		A = GG(A,B,C,D,  X[1],  5, 0xf61e2562, key[5]);
		D = GG(D,A,B,C,  X[6],  9, 0xc040b340, key[5]);
		C = GG(C,D,A,B, X[11], 14, 0x265e5a51, key[5]);
		B = GG(B,C,D,A,  X[0], 20, 0xe9b6c7aa, key[5]);

		A = GG(A,B,C,D,  X[5],  5, 0xd62f105d, key[5]);
		D = GG(D,A,B,C, X[10],  9, 0x02441453, key[5]);
		C = GG(C,D,A,B, X[15], 14, 0xd8a1e681, key[5]);
		B = GG(B,C,D,A,  X[4], 20, 0xe7d3fbc8, key[5]);

		A = GG(A,B,C,D,  X[9],  5, 0x21e1cde6, key[5]);
		D = GG(D,A,B,C, X[14],  9, 0xc33707d6, key[5]);
		C = GG(C,D,A,B,  X[3], 14, 0xf4d50d87, key[5]);
		B = GG(B,C,D,A,  X[8], 20, 0x455a14ed, key[5]);

		A = GG(A,B,C,D, X[13],  5, 0xa9e3e905, key[5]);
		D = GG(D,A,B,C,  X[2],  9, 0xfcefa3f8, key[5]);
		C = GG(C,D,A,B,  X[7], 14, 0x676f02d9, key[5]);
		B = GG(B,C,D,A, X[12], 20, 0x8d2a4c8a, key[5]);

		/* Round 3. */
		/* Let [abcd k s t] denote the operation
		   a = b + ((a + H(b,c,d) + X[k] + T[i]) <<< s). */
		/* Do the following 16 operations. */
		A = HH(A,B,C,D,  X[5],  4, 0xfffa3942, key[6]);
		D = HH(D,A,B,C,  X[8], 11, 0x8771f681, key[6]);
		C = HH(C,D,A,B, X[11], 16, 0x6d9d6122, key[6]);
		B = HH(B,C,D,A, X[14], 23, 0xfde5380c, key[6]);

		A = HH(A,B,C,D,  X[1],  4, 0xa4beea44, key[6]);
		D = HH(D,A,B,C,  X[4], 11, 0x4bdecfa9, key[6]);
		C = HH(C,D,A,B,  X[7], 16, 0xf6bb4b60, key[6]);
		B = HH(B,C,D,A, X[10], 23, 0xbebfbc70, key[6]);

		A = HH(A,B,C,D, X[13],  4, 0x289b7ec6, key[6]);
		D = HH(D,A,B,C,  X[0], 11, 0xeaa127fa, key[6]);
		C = HH(C,D,A,B,  X[3], 16, 0xd4ef3085, key[6]);
		B = HH(B,C,D,A,  X[6], 23, 0x04881d05, key[6]);

		A = HH(A,B,C,D,  X[9],  4, 0xd9d4d039, key[6]);
		D = HH(D,A,B,C, X[12], 11, 0xe6db99e5, key[6]);
		C = HH(C,D,A,B, X[15], 16, 0x1fa27cf8, key[6]);
		B = HH(B,C,D,A,  X[2], 23, 0xc4ac5665, key[6]);

		/* Round 4. */
		/* Let [abcd k s t] denote the operation
		   a = b + ((a + I(b,c,d) + X[k] + T[i]) <<< s). */
		/* Do the following 16 operations. */
		A = II(A,B,C,D,  X[0],  6, 0xf4292244, key[7]);
		D = II(D,A,B,C,  X[7], 10, 0x432aff97, key[7]);
		C = II(C,D,A,B, X[14], 15, 0xab9423a7, key[7]);
		B = II(B,C,D,A,  X[5], 21, 0xfc93a039, key[7]);

		A = II(A,B,C,D, X[12],  6, 0x655b59c3, key[7]);
		D = II(D,A,B,C,  X[3], 10, 0x8f0ccc92, key[7]);
		C = II(C,D,A,B, X[10], 15, 0xffeff47d, key[7]);
		B = II(B,C,D,A,  X[1], 21, 0x85845dd1, key[7]);

		A = II(A,B,C,D,  X[8],  6, 0x6fa87e4f, key[7]);
		D = II(D,A,B,C, X[15], 10, 0xfe2ce6e0, key[7]);
		C = II(C,D,A,B,  X[6], 15, 0xa3014314, key[7]);
		B = II(B,C,D,A, X[13], 21, 0x4e0811a1, key[7]);

		A = II(A,B,C,D,  X[4],  6, 0xf7537e82, key[7]);
		D = II(D,A,B,C, X[11], 10, 0xbd3af235, key[7]);
		C = II(C,D,A,B,  X[2], 15, 0x2ad7d2bb, key[7]);
		B = II(B,C,D,A,  X[9], 21, 0xeb86d391, key[7]);

		/* Then perform the following additions. (That is increment each
		   of the four registers by the value it had before this block
		   was started.) */
		A = A + AA;
		B = B + BB;
		C = C + CC;
		D = D + DD;
	}
	
	public static void main(String[] args)
	{
		
		ByteBuffer key = new ByteBuffer(new byte[] {(byte)0x00,(byte)0x11,(byte)0x22,(byte)0x33,(byte)0x44,(byte)0x55,(byte)0x66,(byte)0x77,(byte)0x88,(byte)0x99,(byte)0xaa,(byte)0xbb,(byte)0xcc,(byte)0xdd,(byte)0xee,(byte)0xff});
		ByteBuffer data = new ByteBuffer("Test\n");
		
		MD5 md5 = new MD5(false);
		md5.setKey(null);
		md5.engineReset();
		md5.engineUpdate(data.getBytes(), 0, data.length());
		ByteBuffer digest2 = new ByteBuffer(md5.engineDigest());
		
		System.out.println(digest2.toHexString(true));
		ByteBuffer shouldBe2 = new ByteBuffer(new byte[] {(byte)0x22,(byte)0x05,(byte)0xe4,(byte)0x8d,(byte)0xe5,(byte)0xf9,(byte)0x3c,(byte)0x78,(byte)0x47,(byte)0x33,(byte)0xff,(byte)0xcc,(byte)0xa8,(byte)0x41,(byte)0xd2,(byte)0xb5});
		System.out.println(shouldBe2.toHexString(true));
		
		System.out.println();
		
		MD5 md5mac = new MD5(true);
		md5mac.setKey(key.getBytes());
		md5mac.engineReset();
		md5mac.engineUpdate(data.getBytes(), 0, data.length());
		ByteBuffer digest = new ByteBuffer(md5mac.engineDigest());
		
		System.out.println(digest.toHexString(true));
		ByteBuffer shouldBe = new ByteBuffer(new byte[] {(byte)0xa6,(byte)0x84,(byte)0xf2,(byte)0x71,(byte)0x0d,(byte)0x61,(byte)0xe7,(byte)0x17,(byte)0x78,(byte)0xe0,(byte)0xc1,(byte)0xb6,(byte)0xdd,(byte)0x1f,(byte)0xa7,(byte)0x28});
		System.out.println(shouldBe.toHexString(true));
		
	}
		
}
