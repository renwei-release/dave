/*
 * Copyright (c) 2001, 2021, Oracle and/or its affiliates. All rights reserved.
 *
 * This file contains the list management definitions for ONS
 */

#ifndef _ORACLE_OPMNLIST_H
#define _ORACLE_OPMNLIST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * List debug build definitions: define OPMN_LIST_DEBUG for validating list
 * management: requires the system "abort()" call
 */
#ifdef OPMN_LIST_DEBUG

# include <stdlib.h>  /* For abort() */

typedef struct _opmn_link_debug
{
  const char*  olndAddFile;  /* File where link was added */
  int          olndAddLine;  /* Line number where link was added */
  const char*  olndRemFile;  /* File where link was removed */
  int          olndRemLine;  /* Line number where link was removed */
  unsigned     olndMagic;    /* Link validation magic number */
} opmn_link_debug;

# define LINK_MAGIC 0x5d3027f1

#endif /* OPMN_LIST_DEBUG */

/*
 * Double linked list management definitions
 */
typedef struct _opmn_link
{
  struct _opmn_link* link_next;
  struct _opmn_link* link_prev;
#ifdef OPMN_LIST_DEBUG
  opmn_link_debug    link_debug;
#endif /* OPMN_LIST_DEBUG */
} opmn_link;

typedef struct _opmn_list
{
  struct _opmn_link* list_head;
  struct _opmn_link* list_tail;
  int                list_count;
} opmn_list;

/*
 * Debug macros for list management
 */
#ifdef OPMN_LIST_DEBUG

# define OPMN_LINK_INIT_DEBUG(_lnk)                                           \
  (_lnk).link_debug.olndAddFile = (const char*)0;                             \
  (_lnk).link_debug.olndAddLine = 0;                                          \
  (_lnk).link_debug.olndRemFile = (const char*)0;                             \
  (_lnk).link_debug.olndRemLine = 0;                                          \
  (_lnk).link_debug.olndMagic = 0;

# define OPMN_LIST_COUNT_DEBUG(_lst)                                          \
  if ((_lst).list_count < 0) abort();

# define OPMN_LINK_ADD_DEBUG(_lnk)                                            \
  if ((_lnk).link_debug.olndMagic != 0) abort();                              \
  (_lnk).link_debug.olndAddFile = __FILE__;                                   \
  (_lnk).link_debug.olndAddLine = __LINE__;                                   \
  (_lnk).link_debug.olndMagic = LINK_MAGIC;

# define OPMN_LINK_REM_DEBUG(_lnk)                                            \
  if ((_lnk).link_debug.olndMagic != LINK_MAGIC) abort();                     \
  (_lnk).link_debug.olndRemFile = __FILE__;                                   \
  (_lnk).link_debug.olndRemLine = __LINE__;                                   \
  (_lnk).link_debug.olndMagic = 0;

# define OPMN_LIST_REM_HEAD_DEBUG(_lst)                                       \
    OPMN_LINK_REM_DEBUG(*((_lst).list_head))

# define OPMN_LIST_REM_TAIL_DEBUG(_lst)                                       \
    OPMN_LINK_REM_DEBUG(*((_lst).list_tail))

#else /* !OPMN_LIST_DEBUG */

# define OPMN_LINK_INIT_DEBUG(_lnk)
# define OPMN_LIST_COUNT_DEBUG(_lst)
# define OPMN_LINK_ADD_DEBUG(_lnk)
# define OPMN_LINK_REM_DEBUG(_lnk)
# define OPMN_LIST_REM_HEAD_DEBUG(_lst)
# define OPMN_LIST_REM_TAIL_DEBUG(_lst)

#endif /* !OPMN_LIST_DEBUG */

/*
 * Macros for list management
 */
#define OPMN_NOLINK                (opmn_link*)0

#define OPMN_LIST_HEAD(_lst)       (_lst).list_head

#define OPMN_LIST_TAIL(_lst)       (_lst).list_tail

#define OPMN_LIST_COUNT(_lst)      (_lst).list_count

#define OPMN_LIST_NEXT_LINK(_lnkP) (_lnkP)->link_next

#define OPMN_LIST_PREV_LINK(_lnkP) (_lnkP)->link_prev

#ifndef offsetof
# define offsetof(_type, _member)  (char*)(&((_type*)0)->_member)
#endif /* offsetof */

#define OPMN_LIST_ELEM(_lnkP, _structName, _linkName)                         \
  (_structName*)((char*)(_lnkP) - offsetof(_structName, _linkName))

#define OPMN_LIST_INIT(_lst)                                                  \
  {                                                                           \
    (_lst).list_head = (opmn_link*)0;                                         \
    (_lst).list_tail = (opmn_link*)0;                                         \
    (_lst).list_count = 0;                                                    \
  }

#define OPMN_LINK_INIT(_lnk)                                                  \
  {                                                                           \
    (_lnk).link_next = (opmn_link*)0;                                         \
    (_lnk).link_prev = (opmn_link*)0;                                         \
    OPMN_LINK_INIT_DEBUG(_lnk)                                                \
  }

#define OPMN_LIST_INS_HEAD(_lst, _lnk)                                        \
  {                                                                           \
    OPMN_LINK_ADD_DEBUG(_lnk)                                                 \
    (_lnk).link_prev = (opmn_link*)0;                                         \
    (_lnk).link_next = (_lst).list_head;                                      \
    if ((_lst).list_head != (opmn_link*)0) {                                  \
      ((_lst).list_head)->link_prev = &(_lnk);                                \
    } else {                                                                  \
      (_lst).list_tail = &(_lnk);                                             \
    }                                                                         \
    (_lst).list_head = &(_lnk);                                               \
    OPMN_LIST_COUNT_DEBUG(_lst)                                               \
    (_lst).list_count++;                                                      \
  }

#define OPMN_LIST_APP_TAIL(_lst, _lnk)                                        \
  {                                                                           \
    OPMN_LINK_ADD_DEBUG(_lnk)                                                 \
    (_lnk).link_next = (opmn_link*)0;                                         \
    (_lnk).link_prev = (_lst).list_tail;                                      \
    if ((_lst).list_tail != (opmn_link*)0) {                                  \
      ((_lst).list_tail)->link_next = &(_lnk);                                \
    } else {                                                                  \
      (_lst).list_head = &(_lnk);                                             \
    }                                                                         \
    (_lst).list_tail = &(_lnk);                                               \
    OPMN_LIST_COUNT_DEBUG(_lst)                                               \
    (_lst).list_count++;                                                      \
  }

#define OPMN_LIST_INS_LINK(_lst, _lnkL, _lnkI)                                \
  {                                                                           \
    OPMN_LINK_ADD_DEBUG(_lnkI)                                                \
    (_lnkI).link_next = &(_lnkL);                                             \
    (_lnkI).link_prev = (_lnkL).link_prev;                                    \
    if ((_lnkL).link_prev != (opmn_link*)0) {                                 \
      ((_lnkL).link_prev)->link_next = &(_lnkI);                              \
    } else {                                                                  \
      (_lst).list_head = &(_lnkI);                                            \
    }                                                                         \
    (_lnkL).link_prev = &(_lnkI);                                             \
    OPMN_LIST_COUNT_DEBUG(_lst)                                               \
    (_lst).list_count++;                                                      \
  }

#define OPMN_LIST_APP_LINK(_lst, _lnkL, _lnkI)                                \
  {                                                                           \
    OPMN_LINK_ADD_DEBUG(_lnkI)                                                \
    (_lnkI).link_prev = &(_lnkL);                                             \
    (_lnkI).link_next = (_lnkL).link_next;                                    \
    if ((_lnkL).link_next != (opmn_link*)0) {                                 \
      ((_lnkL).link_next)->link_prev = &(_lnkI);                              \
    } else {                                                                  \
      (_lst).list_tail = &(_lnkI);                                            \
    }                                                                         \
    (_lnkL).link_next = &(_lnkI);                                             \
    OPMN_LIST_COUNT_DEBUG(_lst)                                               \
    (_lst).list_count++;                                                      \
  }

#define OPMN_LIST_REM_HEAD(_lst)                                              \
  {                                                                           \
    OPMN_LIST_REM_HEAD_DEBUG(_lst)                                            \
    (_lst).list_head = ((_lst).list_head)->link_next;                         \
    if ((_lst).list_head != (opmn_link*)0) {                                  \
      ((_lst).list_head)->link_prev = (opmn_link*)0;                          \
    } else {                                                                  \
      (_lst).list_tail = (opmn_link*)0;                                       \
    }                                                                         \
    (_lst).list_count--;                                                      \
    OPMN_LIST_COUNT_DEBUG(_lst)                                               \
  }

#define OPMN_LIST_REM_TAIL(_lst)                                              \
  {                                                                           \
    OPMN_LIST_REM_TAIL_DEBUG(_lst)                                            \
    (_lst).list_tail = ((_lst).list_tail)->link_prev;                         \
    if ((_lst).list_tail != (opmn_link*)0) {                                  \
      ((_lst).list_tail)->link_next = (opmn_link*)0;                          \
    } else {                                                                  \
      (_lst).list_head = (opmn_link*)0;                                       \
    }                                                                         \
    (_lst).list_count--;                                                      \
    OPMN_LIST_COUNT_DEBUG(_lst)                                               \
  }

#define OPMN_LIST_REM_LINK(_lst, _lnk)                                        \
  {                                                                           \
    OPMN_LINK_REM_DEBUG(_lnk)                                                 \
    if ((_lnk).link_prev != (opmn_link*)0) {                                  \
      ((_lnk).link_prev)->link_next = (_lnk).link_next;                       \
    } else {                                                                  \
      (_lst).list_head = (_lnk).link_next;                                    \
      if ((_lst).list_head != (opmn_link*)0) {                                \
        ((_lst).list_head)->link_prev = (opmn_link*)0;                        \
      }                                                                       \
    }                                                                         \
    if ((_lnk).link_next != (opmn_link*)0) {                                  \
      ((_lnk).link_next)->link_prev = (_lnk).link_prev;                       \
    } else {                                                                  \
      (_lst).list_tail = (_lnk).link_prev;                                    \
      if ((_lst).list_tail != (opmn_link*)0) {                                \
        ((_lst).list_tail)->link_next = (opmn_link*)0;                        \
      }                                                                       \
    }                                                                         \
    (_lst).list_count--;                                                      \
    OPMN_LIST_COUNT_DEBUG(_lst)                                               \
  }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_ORACLE_OPMNLIST_H */
