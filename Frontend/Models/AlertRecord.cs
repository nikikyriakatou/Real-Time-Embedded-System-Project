namespace Models;

public sealed record AlertRecord
{
	public string Outpost { get; set; } = string.Empty;
	public string Alert { get; set; } = string.Empty;
	public double StartingCoordX { get; set; }
	public double StartingCoordY { get; set; }
}
