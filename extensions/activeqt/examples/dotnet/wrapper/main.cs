using System;

namespace WrapperApp
{
	class App
	{
		[STAThread]
		static void Main(string[] args)
		{
			worker.netWorker worker = new worker.netWorker();
			System.Console.Out.WriteLine(worker.StatusString);
			System.Console.Out.WriteLine("Working cycle begins...");

			worker.StatusString = "Working";
			System.Console.Out.WriteLine(worker.StatusString);
			worker.StatusString = "Lunch Break";
			System.Console.Out.WriteLine(worker.StatusString);
			worker.StatusString = "Working";
			System.Console.Out.WriteLine(worker.StatusString);
			worker.StatusString = "Idle";
			System.Console.Out.WriteLine(worker.StatusString);

			System.Console.Out.WriteLine("Working cycle ends...");
		}
	}
}
