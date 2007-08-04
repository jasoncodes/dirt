package au.com.gslabs.dirt.ui.common.client;

import au.com.gslabs.dirt.core.client.*;
import java.util.*;

public class ContactNickCompletor
{
	
	protected Client client;
	
	public ContactNickCompletor(Client client)
	{
		this.client = client;
	}
	
	/**
	 *  Populates <i>candidates</i> with a list of possible
	 *  completions for the <i>buffer</i>. The <i>candidates</i>
	 *  list will not be sorted before being displayed to the
	 *  user: thus, the complete method should sort the
	 *  {@link List} before returning.
	 *
	 *  @param  buffer     the buffer
	 *  @param  candidates the {@link List} of candidates to populate
	 *  @return            the index of the <i>buffer</i> for which
	 *                     the completion will be relative
	 */
	public int complete(final String buffer, final int cursor, final List<String> candidates)
	{
		
		String toFind = (buffer == null) ? "" : buffer;
		toFind = toFind.substring(0, cursor);
		int idxSpace = toFind.lastIndexOf(' ');
		if (idxSpace > -1)
		{
			toFind = toFind.substring(idxSpace+1);
		}
		toFind = toFind.toLowerCase();
		
		String[] nicknames = new String[] { "Jason", "Jack", "Foo", "Bar", "Blacksage" }; //client.getContacts().keys();
		
		for (String nickname : nicknames)
		{
			if (nickname.toLowerCase().startsWith(toFind))
			{
				candidates.add(nickname);
			}
		}
		
		Collections.sort(candidates);
		
		return (candidates.size() == 0) ? (-1) : idxSpace+1;
		
	}
	
}