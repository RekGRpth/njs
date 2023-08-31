
/*
 * Copyright (C) NGINX, Inc.
 */

#ifndef _NJS_FLATHSH_H_INCLUDED_
#define _NJS_FLATHSH_H_INCLUDED_


typedef struct {
    void         *slot;
} njs_flathsh_t;


typedef struct {
    uint32_t     next_elt;
    uint32_t     key_hash;
    void         *value;
} njs_flathsh_elt_t;


typedef struct njs_flathsh_descr_s  njs_flathsh_descr_t;
typedef struct njs_flathsh_query_s  njs_flathsh_query_t;

typedef njs_int_t (*njs_flathsh_test_t)(njs_flathsh_query_t *fhq, void *data);
typedef void *(*njs_flathsh_alloc_t)(void *ctx, size_t size);
typedef void (*njs_flathsh_free_t)(void *ctx, void *p, size_t size);

typedef struct njs_flathsh_proto_s  njs_flathsh_proto_t;


struct njs_flathsh_proto_s {
    uint32_t                   not_used;
    njs_flathsh_test_t         test;
    njs_flathsh_alloc_t        alloc;
    njs_flathsh_free_t         free;
};


struct njs_flathsh_query_s {
    uint32_t                   key_hash;
    njs_str_t                  key;

    uint8_t                    replace;     /* 1 bit */
    void                       *value;

    const njs_flathsh_proto_t  *proto;
    void                       *pool;

    /* Opaque data passed for the test function. */
    void                       *data;
};


#define njs_flathsh_is_empty(fh)                                               \
    ((fh)->slot == NULL)


#define njs_flathsh_init(fh)                                                   \
    (fh)->slot = NULL


#define njs_flathsh_eq(fhl, fhr)                                               \
    ((fhl)->slot == (fhr)->slot)


/*
 * njs_flathsh_find() finds a hash element.  If the element has been
 * found then it is stored in the fhq->value and njs_flathsh_find()
 * returns NJS_OK.  Otherwise NJS_DECLINED is returned.
 *
 * The required njs_flathsh_query_t fields: key_hash, key, proto.
 */
NJS_EXPORT njs_int_t njs_flathsh_find(const njs_flathsh_t *fh,
    njs_flathsh_query_t *fhq);

/*
 * njs_flathsh_insert() adds a hash element.  If the element is already
 * present in flathsh and the fhq->replace flag is zero, then fhq->value
 * is updated with the old element and NJS_DECLINED is returned.
 * If the element is already present in flathsh and the fhq->replace flag
 * is non-zero, then the old element is replaced with the new element.
 * fhq->value is updated with the old element, and NJS_OK is returned.
 * If the element is not present in flathsh, then it is inserted and
 * NJS_OK is returned.  The fhq->value is not changed.
 * On memory allocation failure NJS_ERROR is returned.
 *
 * The required njs_flathsh_query_t fields: key_hash, key, proto, replace,
 * value.
 * The optional njs_flathsh_query_t fields: pool.
 */
NJS_EXPORT njs_int_t njs_flathsh_insert(njs_flathsh_t *fh,
    njs_flathsh_query_t *fhq);

/*
 * njs_flathsh_delete() deletes a hash element.  If the element has been
 * found then it is removed from flathsh and is stored in the fhq->value,
 * and NJS_OK is returned.  Otherwise NJS_DECLINED is returned.
 *
 * The required njs_flathsh_query_t fields: key_hash, key, proto.
 * The optional njs_flathsh_query_t fields: pool.
 */
NJS_EXPORT njs_int_t njs_flathsh_delete(njs_flathsh_t *fh,
    njs_flathsh_query_t *fhq);


typedef struct {
    const njs_flathsh_proto_t  *proto;
    uint32_t                   key_hash;
    uint32_t                   cp;
} njs_flathsh_each_t;


#define njs_flathsh_each_init(lhe, _proto)                                     \
    do {                                                                       \
        njs_memzero(lhe, sizeof(njs_flathsh_each_t));                          \
        (lhe)->proto = _proto;                                                 \
    } while (0)


NJS_EXPORT void *njs_flathsh_each(const njs_flathsh_t *fh,
    njs_flathsh_each_t *fhe);

/*
 * Add element into hash.
 * The element value is not initialized.
 * Returns NULL if memory error in hash expand.
 */
NJS_EXPORT njs_flathsh_elt_t *njs_flathsh_add_elt(njs_flathsh_t *fh,
    njs_flathsh_query_t *fhq);

NJS_EXPORT njs_flathsh_descr_t *njs_flathsh_new(njs_flathsh_query_t *fhq);


/* Temporary backward compatibility .*/

typedef struct njs_flathsh_query_s  njs_lvlhsh_query_t;

#define NJS_LVLHSH_DEFAULT      0
#define NJS_LVLHSH_LARGE_SLAB   0

typedef struct njs_flathsh_proto_s  njs_lvlhsh_proto_t;

#define njs_lvlhsh_is_empty njs_flathsh_is_empty
#define njs_lvlhsh_init njs_flathsh_init
#define njs_lvlhsh_eq njs_flathsh_eq
#define njs_lvlhsh_t njs_flathsh_t
#define njs_lvlhsh_each_t njs_flathsh_each_t
#define njs_lvlhsh_find(lh, lhq) njs_flathsh_find(lh, lhq)
#define njs_lvlhsh_insert(lh, lhq) njs_flathsh_insert(lh, lhq)
#define njs_lvlhsh_delete(lh, lhq) njs_flathsh_delete(lh, lhq)
#define njs_lvlhsh_each_init(lhe, _proto)  njs_flathsh_each_init(lhe, _proto)
#define njs_lvlhsh_each(lh, lhe) njs_flathsh_each(lh, lhe)


#endif /* _NJS_FLATHSH_H_INCLUDED_ */
