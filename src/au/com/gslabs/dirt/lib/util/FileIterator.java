package au.com.gslabs.dirt.lib.util;

/*
Sourced from JMake <http://javamake.sourceforge.net> and brought into the dirt.util package.
Original license comment follows.
*/
/*
	JMake provides a utility similar to UNIX make for use with Java source files 
	but requires no configuration.
	
	Copyright (C) 2002  Jason Weathered, Andrew Marrington
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
import java.util.*;
import java.io.*;

/**
 * <b>Description:</b> FileIterator is used by JMake to process
 * through a directory and return the first source file found.
 *
 * @author Jason Weathered, Andrew Marrington
 * @version 1, 2002-07-28
 *
 * Change History
 * Name                 Date        Version  Description
 * -----------------------------------------------------------------
 * Andrew Marrington &  2002-07-28  1        Created this file.
 * Jason Weathered
 */
public class FileIterator
{

	private String         strDirectories[];
	private String         strFilenames[];
	
	private FilenameFilter filter;
	private String         strDirName;
	private int            iCurrentPos;
	private boolean        bFiles;
	
	private FileIterator   fiRecursive;

	/**
	 * <b>Description:</b> This function prints the complete directory
	 * listing of the directory specified by dirname.
	 *
	 * @param dirname the directory whose files should be recursively 
	 *                processed
	 *
	 * <p><b>Precondition/s:</b> dirname is a valid directory
	 * <br><b>Postcondition/s:</b> ls -r dirname/* (dir /s dirname/*.*)
	 *                             has been printed
	 * <p><b>Complexity:</b> O(n)
	 */	
	public static void main ( String args[] ) 
	{
		FileIterator i = new FileIterator( );
		String strFilename;
		while ( ( strFilename = i.next( ) ) != null )
		{
			System.out.println( strFilename );
		}
	}
	
	/**
	 * <b>Description:</b> This function returns the filename of the
	 * next found file.
	 *
	 * @return the filename of the next file found
	 *
	 * <p><b>Precondition/s:</b> True
	 * <br><b>Postcondition/s:</b> return next file name OR null
	 * and there are no more files
	 * <p><b>Complexity:</b> O(n)
	 */
	public String next( )
	{
		
		if ( fiRecursive != null )
		{
			
			String strReturnVal = fiRecursive.next();
			
			if ( strReturnVal != null )
			{
				return strDirectories[iCurrentPos - 1] + File.separator + strReturnVal;
			}
			else
			{
				fiRecursive = null;
			}
			
		}
		
		if ( !bFiles )
		{
			
			
			if ( iCurrentPos >= strDirectories.length )
			{
				bFiles = true;
				iCurrentPos = 0;
				return next( );
			}
			else
			{
				
				File f = new File ( strDirName + strDirectories[iCurrentPos] );
				iCurrentPos++;
				
				if ( f.isDirectory( ) )
				{
					fiRecursive = new FileIterator ( strDirName + f.getName( ), filter );
					return next( );
				}
				else
				{
					return next( ); // skip file
				}
				
			}
			
			
		}
		else
		{
			
			
			if ( iCurrentPos >= strFilenames.length )
			{
				return null;
			}
			else
			{
				
				File f = new File ( strDirName + strFilenames[iCurrentPos] );
				iCurrentPos++;
	
				if ( !f.isDirectory( ) )
				{
					return f.getName( ); 
				}
				else
				{
					return next( ); // skip directory
				}
	
			}
			
			
		}

	}
	
	/**
	 * <b>Description:</b> Construct a new FileIterator, searching
	 * the directory <code>strDirName</code> and its subdirectories for
	 * files matching <code>filter</code>.
	 *
	 * @param strDirName the top directory from which the search
	 *                   must begin.
	 * @param filter the FilenameFilter by which to filter files
	 *
	 * <p><b>Precondition/s:</b> strDirName is a valid directory
	 * <br><b>Postcondition/s:</b> this exist.
	 */
	public FileIterator ( String strDirName, FilenameFilter filter )
	{

		if ( strDirName.length() == 0 )
		{
			strDirName = ".";
		}

		if ( strDirName.charAt( strDirName.length() - 1 ) != File.separatorChar )
		{
			strDirName += File.separatorChar;
		}
		
		this.strDirName = strDirName;
		this.fiRecursive = null;
		this.filter = filter;
		
		File path = new File(strDirName);
		
		strDirectories = path.list( );
		
		strFilenames = path.list( filter );
		
		iCurrentPos = 0;
		bFiles = false;
		
	}
	
	/**
	 * <b>Description:</b> Construct a new FileIterator, searching
	 * the current directory and its subdirectories for any files.
	 *
	 * <p><b>Precondition/s:</b> True.
	 * <br><b>Postcondition/s:</b> This exists.
	 */
	public FileIterator ( )
	{
		this ( ".", null );
	}
	
}
