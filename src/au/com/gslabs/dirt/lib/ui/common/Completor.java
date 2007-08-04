package au.com.gslabs.dirt.lib.ui.common;

import java.util.List;

public interface Completor
{
	
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
	int complete(final String buffer, final int cursor, final List<String> candidates);
	
}