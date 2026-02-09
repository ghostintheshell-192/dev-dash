using System;
using System.Globalization;
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
                Priority.High => new SolidColorBrush(Color.Parse("#7f1d1d20")),    // red-900/20
                Priority.Medium => new SolidColorBrush(Color.Parse("#78350f20")), // amber-900/20
                Priority.Low => new SolidColorBrush(Color.Parse("#1e293b")),       // slate-800
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
                Priority.High => new SolidColorBrush(Color.Parse("#f87171")),   // red-400
                Priority.Medium => new SolidColorBrush(Color.Parse("#fbbf24")), // amber-400
                Priority.Low => new SolidColorBrush(Color.Parse("#94a3b8")),    // slate-400
                _ => new SolidColorBrush(Color.Parse("#64748b"))
            };
        }
        return new SolidColorBrush(Color.Parse("#64748b"));
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
