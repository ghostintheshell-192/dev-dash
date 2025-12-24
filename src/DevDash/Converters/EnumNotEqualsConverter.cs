using System;
using System.Globalization;
using Avalonia.Data.Converters;

namespace DevDash.Converters;

/// <summary>
/// Converter that compares an enum value with a parameter and returns true if they are NOT equal.
/// Usage: {Binding ActiveTab, Converter={StaticResource EnumNotEqualsConverter}, ConverterParameter={x:Static models:ActiveTabType.Settings}}
/// </summary>
public class EnumNotEqualsConverter : IValueConverter
{
    public static readonly EnumNotEqualsConverter Instance = new();

    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value == null || parameter == null)
            return true;

        // Both should be the same enum type
        if (value.GetType() == parameter.GetType())
        {
            return !value.Equals(parameter);
        }

        return true;
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}
