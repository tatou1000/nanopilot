/*  =========================================================================
    zcertstore - work with CURVE security certificate stores

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of CZMQ, the high-level C binding for 0MQ:
    http://czmq.zeromq.org.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#ifndef __ZCERTSTORE_H_INCLUDED__
#define __ZCERTSTORE_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

//  @warning THE FOLLOWING @INTERFACE BLOCK IS AUTO-GENERATED BY ZPROJECT
//  @warning Please edit the model at "api/zcertstore.api" to make changes.
//  @interface
//  This is a stable class, and may not change except for emergencies. It
//  is provided in stable builds.
//  This class has draft methods, which may change over time. They are not
//  in stable releases, by default. Use --enable-drafts to enable.
//  This class has legacy methods, which will be removed over time. You
//  should not use them, and migrate any code that is still using them.
//  Create a new certificate store from a disk directory, loading and        
//  indexing all certificates in that location. The directory itself may be  
//  absent, and created later, or modified at any time. The certificate store
//  is automatically refreshed on any zcertstore_lookup() call. If the       
//  location is specified as NULL, creates a pure-memory store, which you    
//  can work with by inserting certificates at runtime.                      
CZMQ_EXPORT zcertstore_t *
    zcertstore_new (const char *location);

//  Destroy a certificate store object in memory. Does not affect anything
//  stored on disk.                                                       
CZMQ_EXPORT void
    zcertstore_destroy (zcertstore_t **self_p);

//  Look up certificate by public key, returns zcert_t object if found,
//  else returns NULL. The public key is provided in Z85 text format.  
CZMQ_EXPORT zcert_t *
    zcertstore_lookup (zcertstore_t *self, const char *public_key);

//  Insert certificate into certificate store in memory. Note that this
//  does not save the certificate to disk. To do that, use zcert_save()
//  directly on the certificate. Takes ownership of zcert_t object.    
CZMQ_EXPORT void
    zcertstore_insert (zcertstore_t *self, zcert_t **cert_p);

//  Print list of certificates in store to logging facility
CZMQ_EXPORT void
    zcertstore_print (zcertstore_t *self);

//  *** Deprecated method, slated for removal: avoid using it ***
//  Print list of certificates in store to open stream. This method is
//  deprecated, and you should use the print method.                  
CZMQ_EXPORT void
    zcertstore_fprint (zcertstore_t *self, FILE *file);

//  Self test of this class
CZMQ_EXPORT void
    zcertstore_test (bool verbose);

#ifdef CZMQ_BUILD_DRAFT_API
// Loaders retrieve certificates from an arbitrary source.
typedef void (zcertstore_loader) (
    zcertstore_t *self);

// Destructor for loader state.
typedef void (zcertstore_destructor) (
    void **self_p);

//  *** Draft method, for development use, may change without warning ***
//  Override the default disk loader with a custom loader fn.
CZMQ_EXPORT void
    zcertstore_set_loader (zcertstore_t *self, zcertstore_loader loader, zcertstore_destructor destructor, void *state);

//  *** Draft method, for development use, may change without warning ***
//  Empty certificate hashtable. This wrapper exists to be friendly to bindings,
//  which don't usually have access to struct internals.                        
CZMQ_EXPORT void
    zcertstore_empty (zcertstore_t *self);

#endif // CZMQ_BUILD_DRAFT_API
//  @end


#ifdef __cplusplus
}
#endif

//  Deprecated method aliases
#define zcertstore_dump(s) zcertstore_print(s)

#endif
