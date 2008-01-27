package au.com.gslabs.dirt.ui.jfc.client;

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.core.client.console.*;
import au.com.gslabs.dirt.ui.common.client.ContactNickCompletor;

public class QueryPanel extends BaseClientPanel implements ChatPanel
{
	
	private final MainPanel main;
	private final Contact contact;
	
	private LogPane txtLog;
	private InputArea txtInput;
	
	public QueryPanel(MainPanel main, String context, Contact contact)
	{
		
		super(context);
		this.main = main;
		this.contact = contact;
		
		setLayout(new BorderLayout());
		
		createControls();
		
		new ContainerFocusEventProxy(this).addFocusListener(new FocusListener()
			{
				public void focusLost(FocusEvent e)
				{
					txtLog.clearRedLine();
				}
				public void focusGained(FocusEvent e)
				{
					txtInput.requestFocusInWindow();
				}
			});
			
		getClient().addClientListener(new ClientAdapter()
			{
				@Override
				public void clientContactUpdated(Client source, Contact contact)
				{
					if (getContact().equals(contact))
					{
						notifyTitleChanged();
					}
				}
			}, new JFCInvoker());
		
	}
	
	private void createControls()
	{
		
		txtInput = new InputArea();
		txtInput.setCompletor(new ContactNickCompletor(getClient()));
		txtInput.addInputListener(new InputArea.InputListener()
			{
				
				public void inputPerformed(InputArea source, String[] lines)
				{
					txtInput_Input(lines);
				}
				
				public void inputCompletionCandidates(InputArea source, String[] candidates)
				{
					StringBuilder sb = new StringBuilder();
					sb.append("  ");
					for (String candidate : candidates)
					{
						sb.append("  ");
						if (candidate.indexOf(' ') > -1)
						{
							sb.append('"');
						}
						sb.append(candidate);
						if (candidate.indexOf(' ') > -1)
						{
							sb.append('"');
						}
					}
					txtLog.appendTextLine(sb.toString());
				}
				
				public boolean inputCanAddToHistory(InputArea source, String line)
				{
					return getConsoleClientAdapter().isConsoleInputHistorySafe(line);
				}
				
			});
		add(txtInput, BorderLayout.SOUTH);
		
		txtLog = new LogPane();
		txtLog.addLinkListener(new LogPane.LinkListener()
			{
				public void linkClicked(LogPane.LinkEvent e)
				{
					notifyLinkClicked(e.getURL());
				}
			});
		add(txtLog, BorderLayout.CENTER);
		
		final Color borderColor = getBorderColor();
		final Border borderLineBottom = BorderFactory.createMatteBorder(0,0,1,0, borderColor);
		final Border borderTextArea = BorderFactory.createEmptyBorder(1, 1, 1, FileUtil.isMac()?14:1);
		txtLog.setBorder(borderLineBottom);
		txtInput.setBorder(borderTextArea);
		
	}
	
	private void txtInput_Input(String[] lines)
	{
		txtLog.clearRedLine();
		getConsoleClientAdapter().processConsoleInput(getClient(), getContext(), lines);
	}
	
	public Contact getContact()
	{
		return contact;
	}
	
	public Client getClient()
	{
		return main.getClient();
	}
	
	public ConsoleClientAdapter getConsoleClientAdapter()
	{
		return main.getConsoleClientAdapter();
	}
	
	public String getTitle()
	{
		return contact.getNickname();
	}
	
	public boolean isDirty()
	{
		return false;
	}
	
	public void cleanup()
	{
		main.unregisterPanel(this);
		txtLog.clearText();
	}
	
	public void clearText()
	{
		txtLog.clearText();
	}
	
	public void outputTestData()
	{
		txtLog.addTestData();
		requestAttentionAfterDelay(2000);
	}
	
	public boolean clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message)
	{
		if (!suppressAlert && isDisplayable())
		{
			if (!isFocused())
			{
				txtLog.setRedLine();
				requestAttention();
			}
			if (!txtLog.isAtEnd())
			{
				txtLog.showAndPulseArrow();
			}
		}
		txtLog.appendTextLine(ConsoleClientAdapter.getOutputPrefix() + message, className);
		return true;
	}
	
}