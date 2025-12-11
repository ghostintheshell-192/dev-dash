using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Media;

namespace DevDash.Views.Controls;

public partial class CollapsibleSidebar : UserControl
{
    public static readonly StyledProperty<string?> IconDataProperty =
        AvaloniaProperty.Register<CollapsibleSidebar, string?>(nameof(IconData));

    public static readonly StyledProperty<string?> TooltipTextProperty =
        AvaloniaProperty.Register<CollapsibleSidebar, string?>(nameof(TooltipText));

    public static readonly StyledProperty<object?> SidebarContentProperty =
        AvaloniaProperty.Register<CollapsibleSidebar, object?>(nameof(SidebarContent));

    public static readonly StyledProperty<bool> IsCollapsedProperty =
        AvaloniaProperty.Register<CollapsibleSidebar, bool>(nameof(IsCollapsed), false);

    public static readonly StyledProperty<double> ExpandedWidthProperty =
        AvaloniaProperty.Register<CollapsibleSidebar, double>(nameof(ExpandedWidth), 256);

    private Button? _iconButton;
    private Border? _contentArea;
    private ContentControl? _contentControl;

    public string? IconData
    {
        get => GetValue(IconDataProperty);
        set => SetValue(IconDataProperty, value);
    }

    public string? TooltipText
    {
        get => GetValue(TooltipTextProperty);
        set => SetValue(TooltipTextProperty, value);
    }

    public object? SidebarContent
    {
        get => GetValue(SidebarContentProperty);
        set => SetValue(SidebarContentProperty, value);
    }

    public bool IsCollapsed
    {
        get => GetValue(IsCollapsedProperty);
        set => SetValue(IsCollapsedProperty, value);
    }

    public double ExpandedWidth
    {
        get => GetValue(ExpandedWidthProperty);
        set => SetValue(ExpandedWidthProperty, value);
    }

    public CollapsibleSidebar()
    {
        InitializeComponent();
    }

    protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
    {
        base.OnApplyTemplate(e);

        _iconButton = this.FindControl<Button>("IconButton");
        _contentArea = this.FindControl<Border>("ContentArea");
        _contentControl = this.FindControl<ContentControl>("SidebarContentControl");

        if (_iconButton != null)
        {
            _iconButton.Click += OnIconButtonClicked;
        }

        UpdateIconButton();
        UpdateContentArea();
        UpdateContent();
    }

    protected override void OnPropertyChanged(AvaloniaPropertyChangedEventArgs change)
    {
        base.OnPropertyChanged(change);

        if (change.Property == IconDataProperty || change.Property == TooltipTextProperty)
        {
            UpdateIconButton();
        }
        else if (change.Property == IsCollapsedProperty || change.Property == ExpandedWidthProperty)
        {
            UpdateContentArea();
        }
        else if (change.Property == SidebarContentProperty)
        {
            UpdateContent();
        }
    }

    private void OnIconButtonClicked(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
    {
        IsCollapsed = !IsCollapsed;
        UpdateIconButtonStyle();
    }

    private void UpdateIconButton()
    {
        if (_iconButton == null) return;

        if (!string.IsNullOrEmpty(IconData))
        {
            _iconButton.Content = new PathIcon
            {
                Data = Geometry.Parse(IconData),
                Width = 20,
                Height = 20,
                Foreground = new SolidColorBrush(Color.Parse("#94a3b8"))
            };
        }

        if (!string.IsNullOrEmpty(TooltipText))
        {
            ToolTip.SetTip(_iconButton, TooltipText);
        }

        UpdateIconButtonStyle();
    }

    private void UpdateIconButtonStyle()
    {
        if (_iconButton == null) return;

        if (!IsCollapsed)
        {
            if (!_iconButton.Classes.Contains("active"))
                _iconButton.Classes.Add("active");
        }
        else
        {
            _iconButton.Classes.Remove("active");
        }
    }

    private void UpdateContentArea()
    {
        if (_contentArea == null) return;

        _contentArea.Width = IsCollapsed ? 0 : ExpandedWidth;
        _contentArea.IsVisible = !IsCollapsed;
    }

    private void UpdateContent()
    {
        if (_contentControl == null) return;

        _contentControl.Content = SidebarContent;
    }
}
