using System.Collections.Generic;
using System.Threading.Tasks;
using DevDash.Models;

namespace DevDash.Services;

/// <summary>
/// Servizio per operazioni sul filesystem locale
/// </summary>
public interface IFileSystemService
{
    /// <summary>
    /// Verifica se una directory esiste
    /// </summary>
    bool DirectoryExists(string path);

    /// <summary>
    /// Verifica se un file esiste
    /// </summary>
    bool FileExists(string path);

    /// <summary>
    /// Legge il contenuto di un file
    /// </summary>
    Task<string> ReadFileAsync(string path);

    /// <summary>
    /// Scrive contenuto in un file
    /// </summary>
    Task WriteFileAsync(string path, string content);

    /// <summary>
    /// Elenca le sottodirectory di un path
    /// </summary>
    IEnumerable<string> GetDirectories(string path);

    /// <summary>
    /// Elenca i file in una directory
    /// </summary>
    IEnumerable<string> GetFiles(string path, string pattern = "*");

    /// <summary>
    /// Costruisce l'albero .personal di un progetto
    /// </summary>
    PersonalFile? GetPersonalTree(string projectPath);

    /// <summary>
    /// Ottiene la data di ultima modifica di un file
    /// </summary>
    System.DateTime? GetLastModified(string path);
}
