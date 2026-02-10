using System;
using System.Globalization;
using Avalonia;
using Avalonia.Data.Converters;
using Avalonia.Media;
using DevDash.Models;

namespace DevDash.Converters;

public class PriorityToBrushConverter : IValueConverter
{
    public static readonly PriorityToBrushConverter Instance = new();

    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is Priority priority)
        {
            return priority switch
            {
                Priority.High => PaletteHelper.GetBrush("WarningBg"),
                Priority.Medium => PaletteHelper.GetBrush("AccentBg"),
                Priority.Low => PaletteHelper.GetBrush("BgSecondary"),
                _ => Brushes.Transparent
            };
        }
        return Brushes.Transparent;
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}

public class PriorityToForegroundConverter : IValueConverter
{
    public static readonly PriorityToForegroundConverter Instance = new();

    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is Priority priority)
        {
            return priority switch
            {
                Priority.High => PaletteHelper.GetBrush("Warning"),
                Priority.Medium => PaletteHelper.GetBrush("AccentDim"),
                Priority.Low => PaletteHelper.GetBrush("TextSecondary"),
                _ => PaletteHelper.GetBrush("TextMuted")
            };
        }
        return PaletteHelper.GetBrush("TextMuted");
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}

public class PriorityIsNotNoneConverter : IValueConverter
{
    public static readonly PriorityIsNotNoneConverter Instance = new();

    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is Priority priority)
        {
            return priority != Priority.None;
        }
        return false;
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}
