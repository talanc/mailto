*This project is a work-in-progress and is not ready at this stage*

# TODO List
* Add basic attachment support
* Add wildcard attachment support
* Allow body to be specified from stdin
* Add C# / VB.NET classes
* Make release

# mailto.exe
Simple command line helper for MAPISendMailW

## Rationale
To provide a simple command line tool which can be easily integrated into .NET applications. Calling MAPI functions directly within .NET code is [unsupported](https://blogs.msdn.microsoft.com/mstehle/2007/10/03/fyi-why-are-mapi-and-cdo-1-21-not-supported-in-managed-net-code/).

## Features
* Specify email body, subject, recipients, and attachments.
* Body may be specified as an argument or from standard input.
* Recipients may be To, Cc, or Bcc.
* Attachments also have wildcard support.
* .NET wrapper classes in C# and VB.NET for easy integration.

## Command Line Usage
`mailto.exe [--body TEXT] [--subject TEXT] [--to ADDR...] [--cc ADDR...] [--bcc ADDR...] [--attachment PATH...]`

## Exit codes
| Code | Message
|-:|-
| 0 | Success
| 1 | User cancelled
| > 1 | MAPI Error code (see [Return value](https://msdn.microsoft.com/en-us/library/windows/desktop/hh802867(v=vs.85).aspx))
| < 0 | Error parsing command line arguments, see Standard Error
