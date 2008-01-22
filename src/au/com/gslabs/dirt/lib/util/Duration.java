package au.com.gslabs.dirt.lib.util;

import java.text.NumberFormat;
import java.text.DecimalFormat;
import java.util.Date;

public class Duration implements Comparable<Duration>
{
	
	public enum Precision
	{
		MILLISECONDS,
		SECONDS
	}
	
	public enum OutputFormat
	{
		SHORT,
		MEDIUM,
		LONG
	}
	
	private long milliseconds;
	
	public Duration(Date from, Date to)
	{
		this.milliseconds = to.getTime() - from.getTime();
	}
	
	public Duration(long milliseconds)
	{
		this.milliseconds = milliseconds;
	}
	
	public long getMilliseconds()
	{
		return milliseconds;
	}
	
	public boolean equals(Object obj)
	{
		Duration other = (Duration)obj;
		return this.milliseconds == other.milliseconds;
	}
	
	public int compareTo(Duration other)
	{
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
			if (format == OutputFormat.SHORT)
			{
				result.append(day);
				result.append(':');
			}
			else
			{
				appendVerbose(result, day, (format==OutputFormat.LONG)?"day":"d");
				result.append((format==OutputFormat.LONG)?", ":" ");
			}
		}
		
		if (day!=0 || hour!=0)
		{
			if (format == OutputFormat.SHORT)
			{
				result.append(nf.format(hour));
				result.append(':');
			}
			else
			{
				appendVerbose(result, hour, (format==OutputFormat.LONG)?"hour":"hr");
				result.append((format==OutputFormat.LONG)?", ":" ");
			}
		}
		
		if (precision == Precision.MILLISECONDS && format == OutputFormat.MEDIUM && day==0 && hour==0 && min==0 && sec < 10)
		{
			DecimalFormat df = new DecimalFormat("#,##0");
			result.append(df.format(sec*1000+ms));
			result.append(" ms");
		}
		else
		{
			
			if (format == OutputFormat.SHORT)
			{
				result.append(nf.format(min));
				result.append(':');
				result.append(nf.format(sec));
			}
			else
			{
				if (day!=0 || hour!=0 || min!=0)
				{
					appendVerbose(result, min, (format==OutputFormat.LONG)?"minute":"min");
					result.append((format==OutputFormat.LONG)?", ":" ");
				}
				appendVerbose(result, sec, (format==OutputFormat.LONG)?"second":"sec");
			}
		
			if (precision == Precision.MILLISECONDS)
			{
				if (format == OutputFormat.SHORT)
				{
					nf.setMinimumIntegerDigits(3);
					result.append('.');
					result.append(nf.format(ms));
				}
				else
				{
					if (day!=0 || hour!=0 || min!=0 || sec!=0)
					{
						result.append(", ");
					}
					else
					{
						result = new StringBuffer();
					}
					if (format==OutputFormat.LONG)
					{
						appendVerbose(result, ms, "millisecond");
					}
					else
					{
						result.append(ms);
						result.append(" ms");
					}
				}
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
		System.out.println(new Duration(600).toString(Precision.MILLISECONDS, OutputFormat.MEDIUM));
		System.out.println(new Duration(1200).toString(Precision.MILLISECONDS, OutputFormat.MEDIUM));
		System.out.println(new Duration(1800).toString(Precision.MILLISECONDS, OutputFormat.MEDIUM));
		System.out.println(new Duration(1800).toString(Precision.SECONDS, OutputFormat.MEDIUM));
		System.out.println();
		System.out.println(new Duration(24500).toString(Precision.MILLISECONDS, OutputFormat.MEDIUM));
		System.out.println(new Duration(24500).toString(Precision.SECONDS, OutputFormat.MEDIUM));
		System.out.println();
		System.out.println(new Duration(75300).toString(Precision.MILLISECONDS, OutputFormat.MEDIUM));
		System.out.println(new Duration(75300).toString(Precision.SECONDS, OutputFormat.MEDIUM));
		System.out.println();
	}
	
}
