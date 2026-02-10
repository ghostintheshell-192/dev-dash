using Avalonia;
using Avalonia.Media;

namespace DevDash.Converters;

/// <summary>
/// Resolves brush resources from the centralized ColorPalette.
/// All converters use this instead of hardcoding colors.
/// </summary>
public static class PaletteHelper
{
    public static IBrush GetBrush(string key)
    {
        if (Application.Current?.Resources.TryGetResource(key, Application.Current.ActualThemeVariant, out var resource) == true
            && resource is IBrush brush)
        {
            return brush;
        }

        return Brushes.Transparent;
    }
}
