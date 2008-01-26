package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.core.client.Client;
import au.com.gslabs.dirt.lib.util.TextUtil;
import java.util.ArrayList;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class Preferences
{
	
	public static final String NICKNAME = "nickname";
	public static final String DEFAULT_SERVER = "defaultServer";
	public static final String LOG_CHAT_ENABLED = "logChatEnabled";
	public static final String NOTIFICATION_SOUND_ENABLED = "notificationSoundEnabled";
	
	private static Preferences theInstance = null;
	private final java.util.prefs.Preferences prefs;
	private final ArrayList<ActionListener> listeners = new ArrayList<ActionListener>();
	
	private Preferences()
	{
		prefs = java.util.prefs.Preferences.userNodeForPackage(Preferences.class);
		prefs.addPreferenceChangeListener(new java.util.prefs.PreferenceChangeListener()
			{
				public void preferenceChange(java.util.prefs.PreferenceChangeEvent evt)
				{
					raiseChangeEvent(evt.getKey());
				}
			});
	}
	
	public static Preferences getInstance()
	{
		if (theInstance == null)
		{
			synchronized (Preferences.class)
			{
				if (theInstance == null)
				{
					theInstance = new Preferences();
				}
			}
		}
		return theInstance;
	}
	
	public void addActionListener(ActionListener listener)
	{
		listeners.add(listener);
	}
	
	public void removeActionListener(ActionListener listener)
	{
		int idx = listeners.indexOf(listener);
		listeners.remove(idx);
	}
	
	private void raiseChangeEvent(String key)
	{
		final ActionEvent e = new ActionEvent(this, 0, key.intern());
		for (ActionListener l : listeners)
		{
			l.actionPerformed(e);
		}
	}
	
	public static boolean isDefaultNickname(String nickname)
	{
		return nickname == null || nickname.equals(getDefaultNickname());
	}
	
	public static String getDefaultNickname()
	{
		return Client.getDefaultNickname();
	}
	
	public String getNickname()
	{
		return prefs.get(NICKNAME, null);
	}
	
	public void setNickname(String newValue)
	{
		if (!TextUtil.isEmpty(newValue))
		{
			prefs.put(NICKNAME, newValue);
		}
		else
		{
			prefs.remove(NICKNAME);
		}
	}
	
	public String getDefaultServer()
	{
		return prefs.get(DEFAULT_SERVER, null);
	}
	
	public void setDefaultServer(String newValue)
	{
		if (!TextUtil.isEmpty(newValue))
		{
			prefs.put(DEFAULT_SERVER, newValue);
		}
		else
		{
			prefs.remove(DEFAULT_SERVER);
		}
	}
	
	public boolean getLogChatEnabled()
	{
		return prefs.getBoolean(LOG_CHAT_ENABLED, false);
	}
	
	public void setLogChatEnabled(boolean newValue)
	{
		prefs.putBoolean(LOG_CHAT_ENABLED, newValue);
	}
	
	public boolean getNotificationSoundEnabled()
	{
		return prefs.getBoolean(NOTIFICATION_SOUND_ENABLED, false);
	}
	
	public void setNotificationSoundEnabled(boolean newValue)
	{
		prefs.putBoolean(NOTIFICATION_SOUND_ENABLED, newValue);
	}
	
}
