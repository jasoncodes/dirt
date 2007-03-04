package au.com.gslabs.dirt.lib.util;

import java.util.LinkedList;

// heavily based on:
// http://www-128.ibm.com/developerworks/java/library/j-jtp0730.html

public class WorkQueue
{
	private final int nThreads;
	private final PoolWorker[] threads;
	private final LinkedList<Runnable> queue;

	public WorkQueue(int nThreads)
	{
		this.nThreads = nThreads;
		queue = new LinkedList<Runnable>();
		threads = new PoolWorker[nThreads];

		for (int i=0; i<nThreads; i++) {
			threads[i] = new PoolWorker();
			threads[i].start();
		}
	}

	public void execute(Runnable r) {
		synchronized(queue) {
			queue.addLast(r);
			queue.notify();
		}
	}

	private class PoolWorker extends Thread {
		public void run() {
			Runnable r;

			while (true) {
				synchronized(queue) {
					while (queue.isEmpty()) {
						try
						{
							queue.wait();
						}
						catch (InterruptedException ignored)
						{
						}
					}

					r = queue.removeFirst();
				}

				// If we don't catch RuntimeException, 
				// the pool could leak threads
				try {
					r.run();
				}
				catch (RuntimeException e) {
					// You might want to log something here
					System.err.println(e);
				}
			}
		}
	}
}
