/* 
This code was lifted and hacked up to simply check passwords from
"xlockmore" an extension of "xlock".  The following header is
from the xlock.c module:

 * xlock.c - X11 client to lock a display and show a screen saver.
 *
 * Copyright (c) 1988-91 by Patrick J. Naughton.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
*/


#define PASSLENGTH 64


extern char    *password;


#ifndef VMS
#include <pwd.h>
#endif
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__hpux)
#include <sys/types.h>
#endif
#ifdef ultrix
#include <auth.h>
#endif

#ifdef HP_PASSWDETC
#include <sys/wait.h>
#endif

#ifdef AFS
#include <afs/kauth.h>
#include <afs/kautils.h>
#endif /* AFS */

#if defined(__NetBSD__) || defined(__QNX__) || defined(HPUX_SECURE_PASSWD) || defined(SOLARIS_SHADOW)
#define SECURE_PASSWD
#endif
#include <unistd.h>
#include <string.h>

int
matchesPassword (char *guess, int allowroot)
{
#ifdef VMS
#include <uaidef.h>
#endif

#ifdef AFS
    char *reason;
#endif /* AFS */

    char        *buffer;
#ifndef HP_PASSWDETC
    char        userpass[PASSLENGTH];
#ifdef SOLARIS_SHADOW
    char      user[L_cuserid];
    struct spwd *sp;
#else
    char        rootpass[PASSLENGTH];
    char       *user;
#endif
#endif /* HP_PASSWDETC */
    int          matched;
#ifdef VMS
    char *root = "SYSTEM";
 
    struct itmlst {
      short buflen;
      short code;
      long  addr;
      long  retadr;
    };
    struct itmlst il[4];
 
    short uai_salt,root_salt;
    char hash_password[9], hash_system[9],root_password[9],root_system[9];
    char uai_encrypt,root_encrypt;
 
    struct ascid {
      short  len;
      char dtype;
      char class;
      char *addr;
    };
 
    struct ascid username, password, rootuser;

    user = cuserid(NULL);
    il[0].buflen = 2;
    il[0].code   = UAI$_SALT;
    il[0].addr   = &uai_salt;
    il[0].retadr = 0;
    il[1].buflen = 8;
    il[1].code   = UAI$_PWD;
    il[1].addr   = &hash_password;
    il[1].retadr = 0;
    il[2].buflen = 1;
    il[2].code   = UAI$_ENCRYPT;
    il[2].addr   = &uai_encrypt;
    il[2].retadr = 0;
    il[3].buflen = 0;
    il[3].code   = 0;
    il[3].addr   = 0;
    il[3].retadr = 0;
 
    username.len   = strlen(user);
    username.dtype = 0;
    username.class = 0;
    username.addr  = user;
 
    sys$getuai(0,0,&username,&il,0,0,0);
 
    il[0].buflen = 2;
    il[0].code   = UAI$_SALT;
    il[0].addr   = &root_salt;
    il[0].retadr = 0;
    il[1].buflen = 8;
    il[1].code   = UAI$_PWD;
    il[1].addr   = &root_password;
    il[1].retadr = 0;
    il[2].buflen = 1;
    il[2].code   = UAI$_ENCRYPT;
    il[2].addr   = &root_encrypt;
    il[2].retadr = 0;
    il[3].buflen = 0;
    il[3].code   = 0;
    il[3].addr   = 0;
    il[3].retadr = 0;
 
    rootuser.len   = strlen(root);
    rootuser.dtype = 0;
    rootuser.class = 0;
    rootuser.addr  = root;
 
    sys$getuai(0,0,&rootuser,&il,0,0,0);
#else /* !VMS */
#ifdef OSF1_ENH_SEC
    struct pr_passwd *pw;

    pw = getprpwnam("root");
    (void) strcpy(rootpass, pw->ufld.fd_encrypt);

    pw = getprpwuid (getuid());
    (void) strcpy(userpass, pw->ufld.fd_encrypt);
    user = pw->ufld.fd_name;

#else /* !OSF1_ENH_SEC */
#ifndef HP_PASSWDETC
    struct passwd *pw;

#if defined(SECURE_PASSWD)
    seteuid(0);       /* Need to be root now */
#endif

#ifdef SOLARIS_SHADOW
    /* pw = (struct passwd *)getpwnam("root");
       (void) strcpy(rootpass, pw->pw_passwd);
       pw = (struct passwd *)getpwnam(cuserid(NULL));*/
    sp = getspnam(cuserid(user));
    (void) strcpy(userpass, sp->sp_pwdp);
#else /* !SOLARIS_SHADOW */
    pw = (struct passwd *)getpwnam("root");
    (void) strcpy(rootpass, pw->pw_passwd);

    pw = (struct passwd *)getpwuid(getuid());
#endif /* !SOLARIS_SHADOW */
#if defined(HAS_SHADOW) && defined(linux)
    {
      struct spwd *spw;

      if ((spw = getspnam(pw->pw_name)) == NULL)
        pw->pw_passwd = "";
      else
        pw->pw_passwd = spw->sp_pwdp;
    }
#endif
#ifndef SOLARIS_SHADOW
    (void) strcpy(userpass, pw->pw_passwd);

    user = pw->pw_name;
#endif /* !SOLARIS_SHADOW */
#endif /* !OFS_ENH_SEC */
#if defined(SECURE_PASSWD)
    seteuid(getuid());                /* Don't need to be root anymore */
#endif
#endif /* !HP_PASSWDETC */
#endif /* !VMS */

    matched = 1;

	buffer=guess;

	/*
	 * we don't allow for root to have no password, but we handle the case
	 * where the user has no password correctly; they have to hit return
	 * only
	 */
#ifdef VMS
        password.len   = strlen(&buffer);
        password.dtype = 0;
        password.class = 0;
        password.addr  = &buffer;
 
        str$upcase(&password,&password);
 
        sys$hash_password(&password,uai_encrypt,uai_salt,
                          &username,&hash_system);
        sys$hash_password(&password,root_encrypt,root_salt,
                          &rootuser,&root_system);
        hash_password[8]=0;
        hash_system[8]=0;
        root_password[8]=0;
        root_system[8]=0;
        matched = !((strcmp(hash_password,hash_system)) &&
                 (!allowroot || strcmp(root_password,root_system)));
#else /* !VMS */
#ifdef ultrix
        matched = ((authenticate_user(pw, buffer, NULL) >= 0) ||
        (allowroot && (authenticate_user((struct passwd *) getpwnam("root"),
                       buffer, NULL) >= 0)));
#else /* !ultrix */
#if defined(HAS_SHADOW) && defined(linux)
#ifdef __QNX__
#define pw_encrypt crypt
#endif
        matched = !((strcmp((char *)pw_encrypt(buffer, userpass), userpass)) &&
                 (!allowroot ||
                  strcmp((char *)pw_encrypt(buffer, rootpass), rootpass)));
#else
#ifdef SOLARIS_SHADOW
        matched = !((strcmp(crypt(buffer, userpass), userpass)));
#else /* !SOLARIS_SHADOW */
#ifdef AFS
        /* check afs passwd first, then local, then root */
        matched = !ka_UserAuthenticate(user, "", 0, buffer, 0, &reason);
        if (!matched)
          matched = !((strcmp(crypt(buffer, userpass), userpass)) &&
                   (!allowroot || strcmp(crypt(buffer, rootpass), rootpass)));
#else /* !AFS */

	matched = !((strcmp(crypt(buffer, userpass), userpass)) &&
          (!allowroot || strcmp(crypt(buffer, rootpass), rootpass)));
#endif /* !AFS */
#endif /* !SOLARIS_SHADOW */
#endif
#endif /* !ultrix */
#endif /* !VMS */

#if !defined (ultrix) && !defined (VMS)
        /* userpass is used */
	if (!*userpass && *buffer) {
	    /*
	     * the user has no password, but something was typed anyway.
	     * sounds fishy: don't let him in...
	     */
	    matched = 1;
	}
#endif


    if (matched)
	return (1);
    else
	return (0);

}










