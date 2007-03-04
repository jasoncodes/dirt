package au.com.gslabs.dirt.lib.util;

import java.text.NumberFormat;
import java.util.Date;

public class Duration implements Comparable
{
	
	public enum Precision
	{
		MILLISECONDS,
		SECONDS
	}
	
	public enum OutputFormat
	{
		SHORT,
		LONG
	}
	
	protected long milliseconds;
	
	public Duration(Date from, Date to)
	{
		this.milliseconds = to.getTime() - from.getTime();
	}
	
	public Duration(long milliseconds)
	{
		this.milliseconds = milliseconds;
	}
	
	public boolean equals(Object obj)
	{
		Duration other = (Duration)obj;
		return this.milliseconds == other.milliseconds;
	}
	
	public int compareTo(Object obj)
	{
		Duration other = (Duration)obj;
		if (this.milliseconds == other.milliseconds)
		{
			return 0;
		}
		else
		{
			return (this.milliseconds < other.milliseconds) ? -1 : 1;
		}
	}
	
	public String toString()
	{
		return toString(Precision.SECONDS, OutputFormat.SHORT);
	}
	
	private static void appendVerbose(StringBuffer result, long number, String name)
	{
		result.append(number);
		result.append(' ');
		result.append(name);
		if (number != 1)
		{
			result.append('s');
		}
	}
	
	public String toString(Precision precision, OutputFormat format)
	{
		
		long ms = milliseconds;
		
		boolean neg = ms < 0;
		
		if (neg)
		{
			ms = -ms;
		}
		
		int seconds = (int)( ms / 1000L );
		ms = ms % 1000;
		
		int day = 0, hour, min, sec;
		
		sec = seconds % 60;
		min = seconds / 60;
		
		if (min > 59)
		{
			hour = min / 60;
			min = min % 60;
		}
		else
		{
			hour = 0;
		}
		
		if (hour > 23)
		{
			day = hour / 24;
			hour = hour % 24;
		}
		
		NumberFormat nf = NumberFormat.getNumberInstance();
		nf.setGroupingUsed(false);
		nf.setMinimumIntegerDigits(2);
		
		StringBuffer result = new StringBuffer();
		
		if (day!=0)
		{
			if (format == OutputFormat.LONG)
			{
				appendVerbose(result, day, "day");
				result.append(", ");
			}
			else
			{
				result.append(day);
				result.append(':');
			}
		}
		
		if (day!=0 || hour!=0)
		{
			if (format == OutputFormat.LONG)
			{
				appendVerbose(result, hour, "hour");
				result.append(", ");
			}
			else
			{
				result.append(nf.format(hour));
				result.append(':');
			}
		}
		
		if (format == OutputFormat.LONG)
		{
			if (day!=0 || hour!=0 || min!=0)
			{
				appendVerbose(result, min, "minute");
				result.append(", ");
			}
			appendVerbose(result, sec, "second");
		}
		else
		{
			result.append(nf.format(min));
			result.append(':');
			result.append(nf.format(sec));
		}
		
		if (precision == Precision.MILLISECONDS)
		{
			if (format == OutputFormat.LONG)
			{
				if (day!=0 || hour!=0 || min!=0 || sec!=0)
				{
					result.append(", ");
				}
				else
				{
					result = new StringBuffer();
				}
				appendVerbose(result, ms, "millisecond");
			}
			else
			{
				nf.setMinimumIntegerDigits(3);
				result.append('.');
				result.append(nf.format(ms));
			}
		}
		
		if (neg)
		{
			result.insert(0, '-');
		}
		
		return result.toString();
		
	}
	
	public static void main(String[] args)
	{
		System.out.println();
		System.out.println(new Duration(34567));
		System.out.println(new Duration(34567).toString(Precision.SECONDS, OutputFormat.LONG));
		System.out.println();
		System.out.println(new Duration(34567).toString(Precision.MILLISECONDS, OutputFormat.SHORT));
		System.out.println(new Duration(34567).toString(Precision.MILLISECONDS, OutputFormat.LONG));
		System.out.println();
		System.out.println(new Duration(123456000));
		System.out.println(new Duration(123456000).toString(Precision.SECONDS, OutputFormat.LONG));
		System.out.println();
		System.out.println(new Duration(1002).toString(Precision.MILLISECONDS, OutputFormat.SHORT));
		System.out.println(new Duration(1002).toString(Precision.MILLISECONDS, OutputFormat.LONG));
		System.out.println();
		System.out.println(new Duration(3600000));
		System.out.println(new Duration(3600000).toString(Precision.SECONDS, OutputFormat.LONG));
		System.out.println();
		System.out.println(new Duration(1234000));
		System.out.println(new Duration(1234000).toString(Precision.SECONDS, OutputFormat.LONG));
		System.out.println();
	}
	
}