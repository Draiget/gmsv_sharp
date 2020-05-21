using System;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using Microsoft.CSharp;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;
using Microsoft.CodeAnalysis.Emit;

namespace GmodSharpCore
{
    public class Core
    {
        private static string _baseLibsPath;
        private static string _baseApiLibsPath;
        private static string _baseApiLibName;

        private static readonly IEnumerable<string> DefaultNamespaces =
            new[] {
                "System",
                "System.Linq",
                "System.Text",
                "System.Collections.Generic",
                "GmodApi"
            };

        public static Core Initialize(string modulesLookupPath, string baseLibsPath, string apiLibsPath, string apiLibName) {
            _baseLibsPath = baseLibsPath;
            _baseApiLibsPath = apiLibsPath;
            _baseApiLibName = apiLibName;
            return new Core();
        }

        public static int CompileInline(string code) {
            Console.WriteLine($"CompileInline: {code}");
            try {
                var src = WrapInlineCode(code);
                var libsMeta = new List<MetadataReference> {
                    MetadataReference.CreateFromFile(Path.Combine(_baseLibsPath, "mscorlib.dll")),
                    MetadataReference.CreateFromFile(Path.Combine(_baseLibsPath, "System.dll")),
                    MetadataReference.CreateFromFile(Path.Combine(_baseLibsPath, "System.Core.dll")),
                    MetadataReference.CreateFromFile(Path.Combine(_baseLibsPath, "System.Runtime.dll")),
                    MetadataReference.CreateFromFile(Path.Combine(_baseLibsPath, "System.Private.CoreLib.dll")),
                    MetadataReference.CreateFromFile(Path.Combine(_baseApiLibsPath, _baseApiLibName))
                };
                
                var options =
                    new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary)
                        .WithOverflowChecks(true).WithOptimizationLevel(OptimizationLevel.Release)
                        .WithUsings(DefaultNamespaces);
                
                var assemblyName = Guid.NewGuid().ToString();
                var compilation = CSharpCompilation.Create(assemblyName, new[] {CSharpSyntaxTree.Create(src)}, libsMeta, options);

                Console.WriteLine("Begin compilation ...");
                var watch = Stopwatch.StartNew();
                var assembly = CompileTempAssembly(compilation);
                watch.Stop();
                Console.WriteLine($"Compilation done in {watch.ElapsedMilliseconds} ms...");
                if (assembly == null) {
                    return -1;
                }
                
                var holderClassType = assembly.GetType("GmodModuleExec.TempInlineClass");
                if (holderClassType == null) {
                    return -2;
                }
                
                var execMethod = holderClassType.GetMethod("Exec");
                if (execMethod == null) {
                    return -3;
                }
                
                execMethod.Invoke(null, new object[0]);
            } catch (Exception e) {
                Console.WriteLine($"Error: {e.Message}\n{e.StackTrace}");
                if (e.InnerException != null) {
                    Console.WriteLine($"Inner error: {e.InnerException.Message}\n{e.InnerException.StackTrace}");
                }
                return -11;
            }

            return 0;
        }

        private static Assembly CompileTempAssembly(CSharpCompilation compilation) {
            using var ms = new MemoryStream();
            var result = compilation.Emit(ms);
            if (!ThrowExceptionIfCompilationFailure(result)) {
                return null;
            }

            ms.Seek(0, SeekOrigin.Begin);
            var assembly = AssemblyLoadContext.Default.LoadFromStream(ms);
            return assembly;
        }

        private static bool ThrowExceptionIfCompilationFailure(EmitResult result) {
            if (result.Success) {
                return true;
            }

            var compilationErrors = result.Diagnostics
                .Where(diagnostic =>
                    diagnostic.IsWarningAsError ||
                    diagnostic.Severity == DiagnosticSeverity.Error)
                .ToList();

            if (compilationErrors.Any()) {
                var firstError = compilationErrors.First();
                var errorNumber = firstError.Id;
                var errorDescription = firstError.GetMessage();
                var firstErrorMessage = $"{errorNumber}: {errorDescription};";
                Console.WriteLine($"Compilation failed, first error is: {firstErrorMessage}");
                // throw new Exception($"Compilation failed, first error is: {firstErrorMessage}");
            }

            return false;
        }

        private static CompilationUnitSyntax WrapInlineCode(string code) {
            var syntaxFactory = SyntaxFactory.CompilationUnit();
            
            // Add using statements
            syntaxFactory = syntaxFactory.AddUsings(
                SyntaxFactory.UsingDirective(SyntaxFactory.ParseName("System")),
                SyntaxFactory.UsingDirective(SyntaxFactory.ParseName("GmodApi")));
            
            
            // Create namespace
            var ns = SyntaxFactory.NamespaceDeclaration(SyntaxFactory.ParseName("GmodModuleExec")).NormalizeWhitespace();
            
            
            // Create holder class
            var holderClass = SyntaxFactory.ClassDeclaration("TempInlineClass")
                .WithModifiers(new SyntaxTokenList(SyntaxFactory.Token(SyntaxKind.PublicKeyword), SyntaxFactory.Token(SyntaxKind.StaticKeyword)));
            
            var block = SyntaxFactory.Block(SyntaxFactory.ParseStatement(code).NormalizeWhitespace());
            
            var holderMethod = SyntaxFactory.MethodDeclaration(SyntaxFactory.PredefinedType(SyntaxFactory.Token(SyntaxKind.VoidKeyword)), "Exec")
                .AddModifiers(SyntaxFactory.Token(SyntaxKind.PublicKeyword), SyntaxFactory.Token(SyntaxKind.StaticKeyword))
                .WithBody(block);
            
            holderClass = holderClass.AddMembers(holderMethod);
            ns = ns.AddMembers(holderClass);
            syntaxFactory = syntaxFactory.AddMembers(ns);
            
            var res = syntaxFactory.NormalizeWhitespace();
            return res;
        }

        public static int WrapInlineCode2(string code) {
            var syntaxFactory = SyntaxFactory.CompilationUnit();

            // Add using statements
            syntaxFactory = syntaxFactory.AddUsings(SyntaxFactory.UsingDirective(SyntaxFactory.ParseName("GmodApi")));

            // Create namespace
            var ns = SyntaxFactory.NamespaceDeclaration(SyntaxFactory.ParseName("GmodModuleExec")).NormalizeWhitespace();

            // Create holder class
            var holderClass = SyntaxFactory.ClassDeclaration("Module")
                .AddBaseListTypes(SyntaxFactory.SimpleBaseType(SyntaxFactory.ParseName("SharpEntityBase")))
                .WithModifiers(SyntaxTokenList.Create(SyntaxFactory.Token(SyntaxKind.PublicKeyword)));

            var parsed = CSharpSyntaxTree.ParseText(code);
            var parsedFactory = parsed.GetCompilationUnitRoot();

            foreach (var member in parsedFactory.Members) {
                holderClass.AddMembers(member);
            }

            ns = ns.AddMembers(holderClass);
            syntaxFactory = syntaxFactory.AddMembers(ns);

            var res = syntaxFactory.NormalizeWhitespace().ToFullString();
            Console.WriteLine($"Code: {res}");
            return 0;
        }

        public static int Shutdown() {
            return 0;
        }
    }
}
