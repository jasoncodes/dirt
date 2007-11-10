package au.com.gslabs.dirt.ui.common.client;

import au.com.gslabs.dirt.core.client.*;
import java.util.*;
import au.com.gslabs.dirt.lib.util.Duration;

public class ContactNickCompletor implements au.com.gslabs.dirt.lib.ui.common.Completor
{
	
	private static final long OFFLINE_GRACE_MILLISECONDS = 1000 * 60 * 5; // 5 minutes
	
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
		final int idxSpace = toFind.lastIndexOf(' ');
		if (idxSpace > -1)
		{
			toFind = toFind.substring(idxSpace+1);
		}
		final String toFindLower = toFind.toLowerCase();
		
		int countCaseMatches = 0;
		
		// find nicks which match the prefix (case insensitive)
		for (Contact contact : client.getContacts().values())
		{
			Duration offlineDuration = contact.getOfflineDuration();
			if (offlineDuration == null || offlineDuration.getMilliseconds() <= OFFLINE_GRACE_MILLISECONDS)
			{
				final String thisNick = contact.getNickname();
				if (thisNick.toLowerCase().startsWith(toFindLower))
				{
					candidates.add(contact.getNickname());
					if (thisNick.startsWith(toFind))
					{
						countCaseMatches++;
					}
				}
			}
		}
		
		// if at least one match had the same case as the entered prefix
		// we switch into case sensitive mode by removing all other matches
		if (countCaseMatches > 0 && countCaseMatches < candidates.size())
		{
			for (int i = candidates.size()-1; i >= 0; --i)
			{
				if (!candidates.get(i).startsWith(toFind))
				{
					candidates.remove(i);
				}
			}
		}
		
		Collections.sort(candidates);
		
		return (candidates.size() == 0) ? (-1) : idxSpace+1;
		
	}
	
}