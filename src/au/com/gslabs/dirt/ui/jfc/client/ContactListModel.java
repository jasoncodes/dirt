package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.core.client.Contact;
import au.com.gslabs.dirt.core.client.enums.UserStatus;
import java.util.*;
import javax.swing.*;

public class ContactListModel extends AbstractListModel
{
	
	private class Entry implements Comparable<Entry>
	{
		
		private Contact contact;
		
		private UserStatus key_status;
		private String key_nickname;
		
		public Entry(Contact contact)
		{
			this.contact = contact;
			updateKey();
		}
		
		private void updateKey()
		{
			this.key_status = this.contact.getStatus();
			this.key_nickname = this.contact.getNickname();
		}
		
		public Contact getContact()
		{
			return this.contact;
		}
		
		public int compareTo(Entry other)
		{
			
			if (this == other)
			{
				return 0;
			}
			
			int thisStatusOrder = (this.key_status != UserStatus.OFFLINE) ? 1 : 0;
			int otherStatusOrder = (other.key_status != UserStatus.OFFLINE) ? 1 : 0;
			
			if (thisStatusOrder != otherStatusOrder)
			{
				return thisStatusOrder < otherStatusOrder ? -1 : 1;
			}
			
			return this.key_nickname.compareToIgnoreCase(other.key_nickname);
			
		}
		
		public boolean equals(Object obj)
		{
			Entry other = (Entry)obj;
			return this.compareTo(other) == 0;
		}
		
	}
	
	SortedSet<Entry> entries = new TreeSet<Entry>();
	
	public void updateContact(Contact contact)
	{
		for (Entry entry : entries)
		{
			if (entry.getContact() == contact)
			{
				entries.remove(entry);
				break;
			}
		}
		if (contact.getStatus() != UserStatus.OFFLINE)
		{
			entries.add(new Entry(contact));
		}
		fireContentsChanged(this, 0, getSize());
	}
	
	public Object getElementAt(int index)
	{
		Entry entry = entries.toArray(new Entry[0])[index];
		return entry.getContact();
	}
	
	public int getSize()
	{
		return entries.size();
	}
	
}
