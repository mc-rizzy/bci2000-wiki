/*********************************************************************
 * $Id$
 * Author:      mellinger@neurotechcenter.org
 * Description: C-style interface to the BCI2000 SignalSharingClient
 *   library.
 *   You can use this library to connect to the BCI2000 SignalSharing
 *   interface in order to exchange signal and state data
 *   with a BCI2000 system.
 *
 * $BEGIN_BCI2000_LICENSE$
 *
 * This file is part of BCI2000, a platform for real-time bio-signal research.
 * [ Copyright (C) 2000-2026: BCI2000 team and many external contributors ]
 *
 * BCI2000 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * BCI2000 is distributed in the hope that it will be useful, but
 *                         WITHOUT ANY WARRANTY
 * - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_BCI2000_LICENSE$
 *********************************************************************/
#ifndef SIGNAL_SHARING_CLIENT_LIB_H
#define SIGNAL_SHARING_CLIENT_LIB_H

#ifdef _WIN32
#define STDCALL __stdcall
#ifdef SIGNAL_SHARING_CLIENT_LIBRARY
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif /* SIGNAL_SHARING_CLIENT_LIBRARY */
#elif __GNUC__ && __GNUC__ >= 4
#define STDCALL
#ifdef SIGNAL_SHARING_CLIENT_LIBRARY
#define DLLEXPORT __attribute__((visibility("default")))
#else
#define DLLEXPORT
#endif /* SIGNAL_SHARING_CLIENT_LIBRARY */
#else
#define STDCALL
#define DLLEXPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef enum {
    SSC_SUCCESS = 0,
    SSC_INVALID_ARG,
    SSC_CANNOT_LISTEN,
    SSC_CANNOT_FULFILL_REQUEST,
} SSC_RESULT;

typedef void* SSC_HANDLE;

/* Type for a SignalSharingClient callback function */
typedef void(STDCALL *SSC_CALLBACK)(void* refdata);

/* Create a connector object, and obtain a handle to it. 
   Listening address is in the format IP:port with IP being a name or numeric address.
*/
SSC_RESULT DLLEXPORT STDCALL SSC_CreateConnector(const char* listeningAddress, SSC_HANDLE*);
/* Delete a connector object. Must be called when connector object is no longer used. */
SSC_RESULT DLLEXPORT STDCALL SSC_DeleteConnector(SSC_HANDLE);

/* Register callbacks to be called whenever new signal, states, or parameter data is available. */
SSC_RESULT DLLEXPORT STDCALL SSC_RegisterSignalCallback(SSC_HANDLE, SSC_CALLBACK, void* refdata);
SSC_RESULT DLLEXPORT STDCALL SSC_RegisterStatesCallback(SSC_HANDLE, SSC_CALLBACK, void* refdata);
SSC_RESULT DLLEXPORT STDCALL SSC_RegisterParametersCallback(SSC_HANDLE, SSC_CALLBACK, void* refdata);

/* Lock signal data and obtain signal information. 
   Locking blocks the arrival of new signal data, so call SCC_ReleaseSignal() as soon as possible.
*/
/* All pointer arguments are optional, may be nullptr */
SSC_RESULT DLLEXPORT STDCALL SSC_LockSignal(SSC_HANDLE, const double**, int* channels, int* samples, const char* const** channelNames);
SSC_RESULT DLLEXPORT STDCALL SSC_ReleaseSignal(SSC_HANDLE);

/* Lock state data and obtain state information.
   Locking blocks the arrival of new state data, so call SCC_ReleaseStates() as soon as possible.
*/
/* All pointer arguments are optional, may be nullptr */
SSC_RESULT DLLEXPORT STDCALL SSC_LockStates(SSC_HANDLE, const double**, int* count, int* samples, const char* const** pStateNames);
SSC_RESULT DLLEXPORT STDCALL SSC_ReleaseStates(SSC_HANDLE);

/* Lock signal data and obtain signal information.
   Locking blocks the arrival of new parameter data, so call SCC_ReleaseParameters() as soon as possible.
*/
/* All pointer arguments are optional, may be nullptr */
SSC_RESULT DLLEXPORT STDCALL SSC_LockParameters(SSC_HANDLE, const char* const** parameterLines, int* count);
SSC_RESULT DLLEXPORT STDCALL SSC_ReleaseParameters(SSC_HANDLE);

/* Get signal dimensions for use with PushSignal() */
SSC_RESULT DLLEXPORT STDCALL SSC_GetSignalDimensions(SSC_HANDLE, int*, int*);
/* Push signal data */
SSC_RESULT DLLEXPORT STDCALL SSC_PushSignal(SSC_HANDLE, const double*);
/* Push a state value */
SSC_RESULT DLLEXPORT STDCALL SSC_PushState(SSC_HANDLE, const char*, unsigned int);
/* Commit pushed signal and state data to BCI2000 */
SSC_RESULT DLLEXPORT STDCALL SSC_Commit(SSC_HANDLE);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SIGNAL_SHARING_CLIENT_H */
