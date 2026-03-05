using Firebase.Database;
using Firebase.Database.Query;

namespace Server.Services;

public class FirebaseService
{
	private readonly string _connectionString = "https://tzatziki-5d081-default-rtdb.europe-west1.firebasedatabase.app/";
	private readonly FirebaseClient _firebaseClient;

	public FirebaseService()
	{
		_firebaseClient = new(_connectionString);
	}

	public IObservable<FirebaseObject<T>> SubscribeToTopic<T>(string topic)
	{
		return _firebaseClient.Child(topic).AsObservable<T>();
	}

	public async Task PostEntryToTopic<T>(string topic, T message)
	{
		await _firebaseClient.Child(topic).PostAsync<T>(message);
	}
}
