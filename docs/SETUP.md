# Setup DevDash

Guida per configurare l'ambiente di sviluppo.

---

## Prerequisiti

- **.NET 8 SDK** o superiore
- **Git**
- **IDE consigliato**: VS Code con estensione C# Dev Kit, oppure JetBrains Rider

---

## Installazione

### 1. Clonare il repository

```bash
cd /data/repos
git clone <repo-url> dev-dash
cd dev-dash
```

### 2. Ripristinare dipendenze

```bash
cd src/DevDash
dotnet restore
```

### 3. Compilare

```bash
dotnet build
```

### 4. Eseguire

```bash
dotnet run
```

---

## Struttura progetto

```
dev-dash/
├── src/DevDash/             # Applicazione principale
│   ├── App.axaml            # Application entry point
│   ├── Models/              # Data models (POCO)
│   ├── ViewModels/          # MVVM ViewModels
│   ├── Views/               # XAML views
│   ├── Services/            # Business logic, I/O
│   └── Assets/              # Icone, risorse
├── devdash-prototype.tsx    # Prototipo UI (reference)
├── docs/                    # Documentazione pubblica
├── .personal/               # Note private (non committate)
└── CLAUDE.md                # Istruzioni Claude Code
```

---

## Stack tecnologico

| Layer | Tecnologia |
|-------|------------|
| Framework UI | Avalonia 11.3 |
| Linguaggio | C# / .NET 8 |
| Pattern | MVVM |
| MVVM Toolkit | CommunityToolkit.Mvvm |
| Theme | Avalonia.Themes.Fluent |

---

## Comandi utili

| Comando | Descrizione |
|---------|-------------|
| `dotnet build` | Compila il progetto |
| `dotnet run` | Esegue l'applicazione |
| `dotnet watch run` | Hot reload durante sviluppo |
| `dotnet publish -c Release` | Build per distribuzione |

---

## Configurazione IDE

### VS Code

Estensioni consigliate:

- **C# Dev Kit** - Sviluppo C# completo
- **Avalonia for VS Code** - Supporto XAML Avalonia

Settings consigliati (`.vscode/settings.json`):

```json
{
  "dotnet.defaultSolution": "src/DevDash/DevDash.csproj",
  "editor.formatOnSave": true
}
```

### JetBrains Rider

- Supporto Avalonia built-in
- XAML preview disponibile

---

## Prototipo UI

Il file `devdash-prototype.tsx` contiene il design UI di riferimento in React.

Non è eseguibile direttamente, ma serve come reference per:
- Layout (sidebar, main area, terminal panel)
- Componenti (workspace switcher, file tree, tabs)
- Struttura dati mock (da tradurre in C# Models)

---

## Prossimi passi

Dopo il setup, vedi:

- [ARCHITECTURE.md](ARCHITECTURE.md) per l'architettura
- `.personal/planning/roadmap.md` per il piano di sviluppo
- `.personal/CURRENT-STATUS.md` per lo stato attuale

---

## Troubleshooting

### Errore: SDK version mismatch

Verifica la versione .NET:

```bash
dotnet --version
```

Se < 8.0, aggiorna da https://dotnet.microsoft.com/download

### Errore: package restore failed

```bash
dotnet nuget locals all --clear
dotnet restore
```

### Avalonia non si avvia su Linux

Potrebbe mancare una dipendenza X11:

```bash
# Debian/Ubuntu
sudo apt install libx11-dev
```
