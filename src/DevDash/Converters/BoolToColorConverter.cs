using System;
using System.Globalization;
using Avalonia.Data.Converters;
using Avalonia.Media;

namespace DevDash.Converters;

public class BoolToColorConverter : IValueConverter
{
    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is bool b)
        {
            return b ? new SolidColorBrush(Color.Parse("#22c55e")) : new SolidColorBrush(Color.Parse("#64748b"));
        }
        return new SolidColorBrush(Color.Parse("#64748b"));
    }

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}
