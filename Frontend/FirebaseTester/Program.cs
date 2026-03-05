using Firebase.Database;
using Firebase.Database.Query;
using Models;
using System;
using System.Collections.Concurrent;
using System.Linq;
using System.Threading.Tasks;
public class CustomClass
{
	
	public string Name { get; set; }
	public int Value { get; set; }
}

class Program
{
	static FirebaseClient firebaseClient;
	static ChildQuery firebaseQuery;

	static async Task Main(string[] args)
	{
		// Initialize the Firebase client
		firebaseClient = new FirebaseClient("https://tzatziki-5d081-default-rtdb.europe-west1.firebasedatabase.app/");
		firebaseQuery = firebaseClient.Child("SpeedMessageRecord");

		// Subscribe to changes
		var observable = firebaseQuery.AsObservable<SpeedMessageRecord>();
		observable.Subscribe(d =>
		{
			if (d.Object != null)
			{
				Console.WriteLine($"Received: Key: {d.Key}, Author: {d.Object.Author}, Outpost: {d.Object.Content.Outpost}, MissileId: {d.Object.Content.MissileId}, Speed: {d.Object.Content.Speed}, CoordX: {d.Object.Content.CoordX}, CoordY: {d.Object.Content.CoordY}");
			}
			else
			{
				Console.WriteLine("Received null data");
			}
		});

		Console.WriteLine("Press any key to post a new message...");

		while (true)
		{
			Console.ReadKey();

			// Create an instance of the SpeedMessageRecord class
			var data = new SpeedMessageRecord
			{
				Author = "Test",
				Content = new SpeedRecord
				{
					Outpost = "Outpost1",
					MissileId = new Random().Next(1000),
					Speed = new Random().Next(1000),
					CoordX = new Random().NextDouble(),
					CoordY = new Random().NextDouble()
				}
			};

			// Post the data to Firebase
			var result = await firebaseQuery.PostAsync(data);

			Console.WriteLine($"Posted data with key: {result.Key}");
		}
	}
}
