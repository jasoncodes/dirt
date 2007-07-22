package au.com.gslabs.dirt.lib.crypt.generic;

/**
 * BlockCipher.
 * 
 * @author Christian Plattner, plattner@inf.ethz.ch
 * @version $Id: BlockCipher.java,v 1.1 2007-07-22 23:35:01 jason Exp $
 */
public interface BlockCipher
{
	public void init(boolean forEncryption, byte[] key);

	public int getBlockSize();

	public void transformBlock(byte[] src, int srcoff, byte[] dst, int dstoff);
}
