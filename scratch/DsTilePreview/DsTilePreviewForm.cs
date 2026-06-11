using System.Text;

namespace helengine.ds.scratch.tilepreview;

/// <summary>
/// Shows the cooked glyph atlas rows and packed 8x8 DS tile rows side by side so tile-packing bugs can be debugged visually.
/// </summary>
public sealed class DsTilePreviewForm : Form {
    /// <summary>
    /// Reuses the cooked-font loader that reproduces the DS runtime tile-packing logic.
    /// </summary>
    readonly DsTilePreviewLoader Loader;

    /// <summary>
    /// Holds the path textbox where the user supplies one build workspace or cooked font path.
    /// </summary>
    readonly TextBox PathTextBox;

    /// <summary>
    /// Holds the textbox where the user supplies the character to preview.
    /// </summary>
    readonly TextBox CharacterTextBox;

    /// <summary>
    /// Holds the textbox where the user supplies one text string to preview through consecutive 8x8 tiles.
    /// </summary>
    readonly TextBox PreviewTextTextBox;

    /// <summary>
    /// Toggles whether one 1px outline should be drawn around each 8x8 tile cell in the composed text preview.
    /// </summary>
    readonly CheckBox ShowTileGridCheckBox;

    /// <summary>
    /// Toggles whether the preview should use one direct GDI import instead of the cooked DS font assets from disk.
    /// </summary>
    readonly CheckBox UseDirectGdiCheckBox;

    /// <summary>
    /// Holds the direct GDI source pixel size used when the preview bypasses cooked DS assets.
    /// </summary>
    readonly TextBox DirectGdiPixelSizeTextBox;

    /// <summary>
    /// Triggers one preview reload from the current path and character fields.
    /// </summary>
    readonly Button LoadButton;

    /// <summary>
    /// Replaces the current path with the newest temp DS workspace when one exists.
    /// </summary>
    readonly Button LatestWorkspaceButton;

    /// <summary>
    /// Shows one scaled view of the raw atlas glyph rows.
    /// </summary>
    readonly PictureBox AtlasPictureBox;

    /// <summary>
    /// Shows one scaled view of the packed DS 8x8 tile rows.
    /// </summary>
    readonly PictureBox TilePictureBox;

    /// <summary>
    /// Shows one scaled view of the full 32x24 BG0 tile map with the composed preview text placed into it.
    /// </summary>
    readonly PictureBox FullMapPictureBox;

    /// <summary>
    /// Displays the current load status or the latest failure message.
    /// </summary>
    readonly Label StatusLabel;

    /// <summary>
    /// Displays one textual summary of the selected glyph metrics, paths, and packed tile bytes.
    /// </summary>
    readonly TextBox DetailsTextBox;

    /// <summary>
    /// Creates the preview window and optionally seeds it with one initial workspace path.
    /// </summary>
    /// <param name="initialPath">Initial workspace or cooked font path.</param>
    public DsTilePreviewForm(string initialPath) {
        Loader = new DsTilePreviewLoader();
        Text = "DS Tile Preview";
        Width = 1480;
        Height = 760;
        StartPosition = FormStartPosition.CenterScreen;

        PathTextBox = new TextBox();
        CharacterTextBox = new TextBox();
        PreviewTextTextBox = new TextBox();
        ShowTileGridCheckBox = new CheckBox();
        UseDirectGdiCheckBox = new CheckBox();
        DirectGdiPixelSizeTextBox = new TextBox();
        LoadButton = new Button();
        LatestWorkspaceButton = new Button();
        AtlasPictureBox = new PictureBox();
        TilePictureBox = new PictureBox();
        FullMapPictureBox = new PictureBox();
        StatusLabel = new Label();
        DetailsTextBox = new TextBox();

        InitializeControls(initialPath);
        Load += HandleFormLoad;
    }

    /// <summary>
    /// Configures the window controls and layout.
    /// </summary>
    /// <param name="initialPath">Initial workspace or cooked font path.</param>
    void InitializeControls(string initialPath) {
        TableLayoutPanel rootLayout = new TableLayoutPanel();
        rootLayout.Dock = DockStyle.Fill;
        rootLayout.ColumnCount = 1;
        rootLayout.RowCount = 4;
        rootLayout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        rootLayout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        rootLayout.RowStyles.Add(new RowStyle(SizeType.Percent, 100.0f));
        rootLayout.RowStyles.Add(new RowStyle(SizeType.Absolute, 240.0f));

        FlowLayoutPanel topPanel = new FlowLayoutPanel();
        topPanel.Dock = DockStyle.Fill;
        topPanel.AutoSize = true;
        topPanel.WrapContents = false;

        Label pathLabel = new Label();
        pathLabel.Text = "Workspace / Font:";
        pathLabel.Width = 120;
        pathLabel.TextAlign = ContentAlignment.MiddleLeft;

        PathTextBox.Width = 560;
        PathTextBox.Text = initialPath ?? string.Empty;

        LatestWorkspaceButton.Text = "Latest";
        LatestWorkspaceButton.AutoSize = true;
        LatestWorkspaceButton.Click += HandleLatestWorkspaceButtonClick;

        Label characterLabel = new Label();
        characterLabel.Text = "Glyph:";
        characterLabel.Width = 48;
        characterLabel.TextAlign = ContentAlignment.MiddleLeft;

        CharacterTextBox.Width = 48;
        CharacterTextBox.Text = "H";

        Label previewTextLabel = new Label();
        previewTextLabel.Text = "Text:";
        previewTextLabel.Width = 36;
        previewTextLabel.TextAlign = ContentAlignment.MiddleLeft;

        PreviewTextTextBox.Width = 180;
        PreviewTextTextBox.Text = "Update";

        ShowTileGridCheckBox.Text = "Show 8x8 Grid";
        ShowTileGridCheckBox.AutoSize = true;
        ShowTileGridCheckBox.Checked = true;

        UseDirectGdiCheckBox.Text = "Use Direct GDI";
        UseDirectGdiCheckBox.AutoSize = true;
        UseDirectGdiCheckBox.Checked = true;

        Label directGdiPixelSizeLabel = new Label();
        directGdiPixelSizeLabel.Text = "Px:";
        directGdiPixelSizeLabel.Width = 20;
        directGdiPixelSizeLabel.TextAlign = ContentAlignment.MiddleLeft;

        DirectGdiPixelSizeTextBox.Width = 40;
        DirectGdiPixelSizeTextBox.Text = "8";

        LoadButton.Text = "Load";
        LoadButton.AutoSize = true;
        LoadButton.Click += HandleLoadButtonClick;

        topPanel.Controls.Add(pathLabel);
        topPanel.Controls.Add(PathTextBox);
        topPanel.Controls.Add(LatestWorkspaceButton);
        topPanel.Controls.Add(characterLabel);
        topPanel.Controls.Add(CharacterTextBox);
        topPanel.Controls.Add(previewTextLabel);
        topPanel.Controls.Add(PreviewTextTextBox);
        topPanel.Controls.Add(UseDirectGdiCheckBox);
        topPanel.Controls.Add(directGdiPixelSizeLabel);
        topPanel.Controls.Add(DirectGdiPixelSizeTextBox);
        topPanel.Controls.Add(ShowTileGridCheckBox);
        topPanel.Controls.Add(LoadButton);

        StatusLabel.Dock = DockStyle.Fill;
        StatusLabel.Height = 28;
        StatusLabel.TextAlign = ContentAlignment.MiddleLeft;
        StatusLabel.Text = "Ready.";

        TableLayoutPanel previewLayout = new TableLayoutPanel();
        previewLayout.Dock = DockStyle.Fill;
        previewLayout.ColumnCount = 3;
        previewLayout.RowCount = 2;
        previewLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 26.0f));
        previewLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 30.0f));
        previewLayout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 44.0f));
        previewLayout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        previewLayout.RowStyles.Add(new RowStyle(SizeType.Percent, 100.0f));

        Label atlasLabel = new Label();
        atlasLabel.Text = "Atlas Glyph";
        atlasLabel.Dock = DockStyle.Fill;
        atlasLabel.TextAlign = ContentAlignment.MiddleCenter;

        Label tileLabel = new Label();
        tileLabel.Text = "Packed Text Preview";
        tileLabel.Dock = DockStyle.Fill;
        tileLabel.TextAlign = ContentAlignment.MiddleCenter;

        Label fullFontMapLabel = new Label();
        fullFontMapLabel.Text = "Full 32x24 Font Tile Map";
        fullFontMapLabel.Dock = DockStyle.Fill;
        fullFontMapLabel.TextAlign = ContentAlignment.MiddleCenter;

        AtlasPictureBox.Dock = DockStyle.Fill;
        AtlasPictureBox.BackColor = Color.Black;
        AtlasPictureBox.SizeMode = PictureBoxSizeMode.CenterImage;

        TilePictureBox.Dock = DockStyle.Fill;
        TilePictureBox.BackColor = Color.Black;
        TilePictureBox.SizeMode = PictureBoxSizeMode.CenterImage;

        FullMapPictureBox.Dock = DockStyle.Fill;
        FullMapPictureBox.BackColor = Color.Black;
        FullMapPictureBox.SizeMode = PictureBoxSizeMode.CenterImage;

        previewLayout.Controls.Add(atlasLabel, 0, 0);
        previewLayout.Controls.Add(tileLabel, 1, 0);
        previewLayout.Controls.Add(fullFontMapLabel, 2, 0);
        previewLayout.Controls.Add(AtlasPictureBox, 0, 1);
        previewLayout.Controls.Add(TilePictureBox, 1, 1);
        previewLayout.Controls.Add(FullMapPictureBox, 2, 1);

        DetailsTextBox.Dock = DockStyle.Fill;
        DetailsTextBox.Multiline = true;
        DetailsTextBox.ReadOnly = true;
        DetailsTextBox.ScrollBars = ScrollBars.Both;
        DetailsTextBox.Font = new Font("Consolas", 10.0f, FontStyle.Regular, GraphicsUnit.Point);

        rootLayout.Controls.Add(topPanel, 0, 0);
        rootLayout.Controls.Add(StatusLabel, 0, 1);
        rootLayout.Controls.Add(previewLayout, 0, 2);
        rootLayout.Controls.Add(DetailsTextBox, 0, 3);

        Controls.Add(rootLayout);
    }

    /// <summary>
    /// Loads the initial preview once the window becomes visible.
    /// </summary>
    /// <param name="sender">Raised form instance.</param>
    /// <param name="e">Unused event payload.</param>
    void HandleFormLoad(object sender, EventArgs e) {
        if (!string.IsNullOrWhiteSpace(PathTextBox.Text)) {
            LoadPreview();
        }
    }

    /// <summary>
    /// Replaces the current path with the newest temp DS workspace path.
    /// </summary>
    /// <param name="sender">Clicked button instance.</param>
    /// <param name="e">Unused event payload.</param>
    void HandleLatestWorkspaceButtonClick(object sender, EventArgs e) {
        PathTextBox.Text = LatestDsWorkspaceLocator.ResolveInitialPath();
    }

    /// <summary>
    /// Starts one preview reload using the current form field values.
    /// </summary>
    /// <param name="sender">Clicked button instance.</param>
    /// <param name="e">Unused event payload.</param>
    void HandleLoadButtonClick(object sender, EventArgs e) {
        LoadPreview();
    }

    /// <summary>
    /// Loads one preview result and updates every visual section of the form.
    /// </summary>
    void LoadPreview() {
        try {
            char character = ResolveRequestedCharacter();
            DsTilePreviewResult result;
            DsTilePreviewTextResult textResult;
            if (UseDirectGdiCheckBox.Checked) {
                float directGdiPixelSize = ResolveDirectGdiPixelSize();
                result = Loader.LoadDirectGdi("Consolas", directGdiPixelSize, character);
                textResult = Loader.LoadDirectGdiTextPreview("Consolas", directGdiPixelSize, PreviewTextTextBox.Text ?? string.Empty);
            } else {
                result = Loader.Load(PathTextBox.Text, character);
                textResult = Loader.LoadTextPreview(PathTextBox.Text, PreviewTextTextBox.Text ?? string.Empty);
            }
            AtlasPictureBox.Image = RenderBitmap(result.AtlasRows, 24, 6);
            TilePictureBox.Image = RenderBitmap(textResult.Rows, Math.Max(textResult.Rows[0].Length, 8), 8, ShowTileGridCheckBox.Checked);
            FullMapPictureBox.Image = RenderBitmap(textResult.FullFontMapRows, 32 * 8, 3, ShowTileGridCheckBox.Checked);
            DetailsTextBox.Text = BuildDetails(result, textResult);
            StatusLabel.Text = $"Loaded '{result.Character}' from {result.FontPath}";
        } catch (Exception exception) {
            StatusLabel.Text = exception.Message;
            AtlasPictureBox.Image = null;
            TilePictureBox.Image = null;
            FullMapPictureBox.Image = null;
            DetailsTextBox.Text = exception.ToString();
        }
    }

    /// <summary>
    /// Resolves the single preview character requested by the user.
    /// </summary>
    /// <returns>Single character selected for preview.</returns>
    char ResolveRequestedCharacter() {
        string requestedCharacterText = CharacterTextBox.Text == null ? string.Empty : CharacterTextBox.Text.Trim();
        if (requestedCharacterText.Length != 1) {
            throw new InvalidOperationException("Glyph field must contain exactly one character.");
        }

        return requestedCharacterText[0];
    }

    /// <summary>
    /// Resolves the direct GDI preview pixel size from the form field.
    /// </summary>
    /// <returns>Positive direct GDI preview pixel size.</returns>
    float ResolveDirectGdiPixelSize() {
        string directGdiPixelSizeText = DirectGdiPixelSizeTextBox.Text == null ? string.Empty : DirectGdiPixelSizeTextBox.Text.Trim();
        if (!float.TryParse(directGdiPixelSizeText, out float directGdiPixelSize) || directGdiPixelSize <= 0.0f) {
            throw new InvalidOperationException("Direct GDI pixel size must be one positive number.");
        }

        return directGdiPixelSize;
    }

    /// <summary>
    /// Renders one text-grid preview into one enlarged bitmap using white for visible pixels and black for empty pixels.
    /// </summary>
    /// <param name="rows">Text rows composed of <c>#</c> and <c>.</c>.</param>
    /// <param name="columnCount">Logical column count to render.</param>
    /// <param name="pixelScale">Scale factor used for the output bitmap.</param>
    /// <returns>Rendered bitmap for the requested text-grid preview.</returns>
    Bitmap RenderBitmap(string[] rows, int columnCount, int pixelScale, bool drawTileGrid = false) {
        if (rows == null || rows.Length == 0) {
            throw new InvalidOperationException("Preview rows were not produced.");
        }

        Bitmap bitmap = new Bitmap(columnCount * pixelScale, rows.Length * pixelScale);
        using Graphics graphics = Graphics.FromImage(bitmap);
        graphics.Clear(Color.Black);
        using SolidBrush brush = new SolidBrush(Color.White);
        using Pen gridPen = new Pen(Color.FromArgb(96, 96, 96), 1.0f);
        for (int y = 0; y < rows.Length; y++) {
            string row = rows[y] ?? string.Empty;
            for (int x = 0; x < row.Length; x++) {
                if (row[x] == '#') {
                    graphics.FillRectangle(brush, x * pixelScale, y * pixelScale, pixelScale, pixelScale);
                }
            }
        }

        if (drawTileGrid) {
            int tileColumnCount = columnCount / 8;
            int tileRowCount = rows.Length / 8;
            for (int tileRowIndex = 0; tileRowIndex < tileRowCount; tileRowIndex++) {
                for (int tileColumnIndex = 0; tileColumnIndex < tileColumnCount; tileColumnIndex++) {
                    graphics.DrawRectangle(
                        gridPen,
                        tileColumnIndex * 8 * pixelScale,
                        tileRowIndex * 8 * pixelScale,
                        (8 * pixelScale) - 1,
                        (8 * pixelScale) - 1);
                }
            }
        }

        return bitmap;
    }

    /// <summary>
    /// Builds one textual summary of the current preview result.
    /// </summary>
    /// <param name="result">Preview result to summarize.</param>
    /// <returns>Multi-line diagnostic summary for the loaded glyph.</returns>
    string BuildDetails(DsTilePreviewResult result, DsTilePreviewTextResult textResult) {
        StringBuilder builder = new StringBuilder();
        builder.AppendLine($"FontPath: {result.FontPath}");
        builder.AppendLine($"AtlasPath: {result.AtlasPath}");
        builder.AppendLine($"Glyph: {result.Character}");
        builder.AppendLine($"PreviewText: {textResult.Text}");
        builder.AppendLine($"FullFontMapGlyphCount: {textResult.FullFontMapGlyphCount}");
        builder.AppendLine($"FullFontMapTileColumns: {textResult.FullFontMapTileColumns}");
        builder.AppendLine($"Atlas: {result.AtlasWidth}x{result.AtlasHeight}");
        builder.AppendLine($"Rect: X={result.SourceX} Y={result.SourceY} W={result.SourceWidth} H={result.SourceHeight}");
        builder.AppendLine($"Advance: {result.AdvanceWidth}");
        builder.AppendLine($"BearingX: {result.BearingX}");
        builder.AppendLine($"BearingY: {result.BearingY}");
        builder.AppendLine();
        builder.AppendLine("AtlasRows:");
        foreach (string row in result.AtlasRows) {
            builder.AppendLine(row);
        }

        builder.AppendLine();
        builder.AppendLine("TileRows:");
        foreach (string row in result.TileRows) {
            builder.AppendLine(row);
        }

        builder.AppendLine();
        builder.AppendLine("TileBytes:");
        builder.AppendLine(string.Join(",", result.TileBytes.Select(value => value.ToString("X2"))));
        builder.AppendLine();
        builder.AppendLine("PreviewRows:");
        foreach (string row in textResult.Rows) {
            builder.AppendLine(row);
        }
        builder.AppendLine();
        builder.AppendLine("FullFontMapRows:");
        foreach (string row in textResult.FullFontMapRows) {
            builder.AppendLine(row);
        }
        return builder.ToString();
    }
}
