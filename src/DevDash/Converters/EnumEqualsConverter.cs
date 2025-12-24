using System;
using System.Globalization;
using Avalonia.Data.Converters;

namespace DevDash.Converters;

/// <summary>
/// Converter that compares an enum value with a parameter and returns true if they are equal.
/// Usage: {Binding ActiveTab, Converter={StaticResource EnumEqualsConverter}, ConverterParameter={x:Static models:ActiveTabType.Settings}}
/// </summary>
public class EnumEqualsConverter : IValueConverter
{
    public static readonly EnumEqualsConverter Instance = new();

    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value == null || parameter == null)
            return false;

        // Both should be the same enum type
        if (value.GetType() == parameter.GetType())
        {
            return value.Equals(parameter);
        }

        return false;
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}
