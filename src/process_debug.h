// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#ifndef PROCESS_DEBUG_H
#define PROCESS_DEBUG_H

extern bool verbose_flag;
extern bool regress_flag;

extern int exit_status;

#define regress(prg) { if (regress_flag) { prg } }
#define verbose(prg) { if (verbose_flag || regress_flag) { prg } }
#define verbose_only(prg) { if (verbose_flag && !regress_flag) { prg } }
#define verbose_regress(prg) { if (verbose_flag && regress_flag) { prg } }

#endif // PROCESS_DEBUG_H
