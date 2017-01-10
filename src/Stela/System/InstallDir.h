#ifndef INSTALLDIR_H
#define INSTALLDIR_H

NSMAKE_CMD( "process.stdout.write( `#define INSTALL_DIR \"${ process.cwd() }\"` )", ".js" );

#endif // INSTALLDIR_H
