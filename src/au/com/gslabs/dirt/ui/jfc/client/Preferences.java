package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.core.client.Client;
import au.com.gslabs.dirt.lib.util.TextUtil;
import java.util.ArrayList;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Rectangle;

public class Preferences
{
	
	public static final String NICKNAME = "nickname";
	public static final String DEFAULT_SERVER = "defaultServer";
	public static final String LOG_CHAT_ENABLED = "logChatEnabled";
	public static final String NOTIFICATION_SOUND_ENABLED = "notificationSoundEnabled";
	public static final String LASTREADMARKER_CLEAR_BLUR = "lastReadMarkerClearOnBlur";
	public static final String LASTREADMARKER_CLEAR_INPUT = "lastReadMarkerClearOnInput";
	
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
	
	public boolean getLastReadMarkerClearOnBlur()
	{
		return prefs.getBoolean(LASTREADMARKER_CLEAR_BLUR, true);
	}
	
	public void setLastReadMarkerClearOnBlur(boolean newValue)
	{
		prefs.putBoolean(LASTREADMARKER_CLEAR_BLUR, newValue);
	}
	
	public boolean getLastReadMarkerClearOnInput()
	{
		return prefs.getBoolean(LASTREADMARKER_CLEAR_INPUT, false);
	}
	
	public void setLastReadMarkerClearOnInput(boolean newValue)
	{
		prefs.putBoolean(LASTREADMARKER_CLEAR_INPUT, newValue);
	}
	
	protected java.util.prefs.Preferences[] getPanelPreferences(String[] panelKeys, boolean createIfNotExist)
	{
		
		final ArrayList<java.util.prefs.Preferences> selectedPrefs = new ArrayList<java.util.prefs.Preferences>();
		
		final java.util.prefs.Preferences panelPrefs = prefs.node("panels");
		for (final String panelKey : panelKeys)
		{
			if (createIfNotExist || containsNode(panelPrefs, panelKey))
			{
				final java.util.prefs.Preferences panelPref = panelPrefs.node(panelKey);
				if (!selectedPrefs.contains(panelPref))
				{
					selectedPrefs.add(panelPref);
				}
			}
		}
		
		return selectedPrefs.toArray(new java.util.prefs.Preferences[0]);
		
	}
	
	protected static boolean arrayContains(Object[] haystack, Object needle)
	{
		for (Object item : haystack)
		{
			if ((needle == null && item == null) || (needle != null && needle.equals(item)))
			{
				return true;
			}
		}
		return false;
	}
	
	protected static boolean containsKey(java.util.prefs.Preferences prefs, String key)
	{
		try
		{
			return arrayContains(prefs.keys(), key);
		}
		catch (java.util.prefs.BackingStoreException ex)
		{
			System.err.println("Preferences retrieval error:");
			ex.printStackTrace();
			return false;
		}
	}
	
	protected static boolean containsNode(java.util.prefs.Preferences prefs, String key)
	{
		try
		{
			return prefs.nodeExists(key);
		}
		catch (java.util.prefs.BackingStoreException ex)
		{
			System.err.println("Preferences retrieval error:");
			ex.printStackTrace();
			return false;
		}
	}
	
	public int getPanelInt(String[] panelKeys, String prefKey, int defaultValue)
	{
		
		for (java.util.prefs.Preferences panelPref : getPanelPreferences(panelKeys, false))
		{
			if (containsKey(panelPref, prefKey))
			{
				return panelPref.getInt(prefKey, defaultValue);
			}
		}
		
		return defaultValue;
		
	}
	
	public void setPanelInt(String[] panelKeys, String prefKey, int value)
	{
		for (java.util.prefs.Preferences panelPref : getPanelPreferences(panelKeys, true))
		{
			panelPref.putInt(prefKey, value);
		}
	}
	
	public Rectangle getPanelRectangle(String[] panelKeys, String prefKey, Rectangle defaultValue)
	{
		for (java.util.prefs.Preferences panelPref : getPanelPreferences(panelKeys, false))
		{
			if (containsNode(panelPref, prefKey))
			{
				java.util.prefs.Preferences rectPref = panelPref.node(prefKey);
				if (containsKey(rectPref, "x") && containsKey(rectPref, "y") &&
					containsKey(rectPref, "width") && containsKey(rectPref, "height"))
				{
					Rectangle rect = new Rectangle();
					rect.x = rectPref.getInt("x", Integer.MIN_VALUE);
					rect.y = rectPref.getInt("y", Integer.MIN_VALUE);
					rect.width = rectPref.getInt("width", Integer.MIN_VALUE);
					rect.height = rectPref.getInt("height", Integer.MIN_VALUE);
					if (rect.x != Integer.MIN_VALUE && rect.y != Integer.MIN_VALUE &&
						rect.width != Integer.MIN_VALUE && rect.height != Integer.MIN_VALUE)
					{
						return rect;
					}
				}
			}
		}
		return defaultValue;
	}
	
	public void setPanelRectangle(String[] panelKeys, String prefKey, Rectangle value)
	{
		for (java.util.prefs.Preferences panelPref : getPanelPreferences(panelKeys, true))
		{
			java.util.prefs.Preferences rectPref = panelPref.node(prefKey);
			rectPref.putInt("x", value.x);
			rectPref.putInt("y", value.y);
			rectPref.putInt("width", value.width);
			rectPref.putInt("height", value.height);
		}
	}
	
}
