namespace Models;
public sealed record SpeedRecord
{
	public string Outpost { get; set; } = string.Empty;

	public long MissileId { get; set; }

	public long Speed { get; set; }

	public double CoordX { get; set; }

	public double CoordY { get; set; }
}
