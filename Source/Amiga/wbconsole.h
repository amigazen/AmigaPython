/*
 * wbconsole.h - Workbench console and argv setup for VBCC/PosixLib
 *
 * SAS/C used __stdiowin to auto-open CON: from Workbench. PosixLib does
 * not; Workbench launches have pr_CIS/pr_COS ZERO. Open one CON:, wire
 * SelectInput/SelectOutput to that single FH, clear all PRF_CLOSE* flags,
 * and Close the handle exactly once on cleanup (RKR M DOS 5.3.2).
 */

#ifndef AMIGA_WBCONSOLE_H
#define AMIGA_WBCONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Returns 1 if this process was started from Workbench (no CLI).
 */
int amiga_from_workbench(void);

/*
 * If started from Workbench: open CON:, wire stdio, rebuild *argc/*argv
 * for Py_Main. Requires lib/python27.zip beside the tool or Assign Python:.
 * If started from a Shell: leave argc/argv unchanged.
 * Returns 0 on success, -1 on failure (caller should exit).
 */
int amiga_wb_prepare(int *argc, char ***argv);

/*
 * Detach stdio from CON: and Close the FH once. Safe if unused.
 */
void amiga_wb_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* AMIGA_WBCONSOLE_H */
