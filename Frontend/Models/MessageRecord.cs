namespace Models;
public record MessageRecord
{
	public string Author { get; set; } = string.Empty;
}

public sealed record AlertMessageRecord : MessageRecord
{
	public AlertRecord Content { get; set; }
}

public sealed record SpeedMessageRecord : MessageRecord
{
	public SpeedRecord Content { get; set; }
}


public sealed record Message
{
	public string Author { get; set; }
	public string Content { get; set; }
}