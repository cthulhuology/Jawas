#ifndef jsxml_h___
#define jsxml_h___

#include "jsstddef.h"
#include "jspubtd.h"

extern const char js_AnyName_str[];
extern const char js_AttributeName_str[];
extern const char js_isXMLName_str[];
extern const char js_XMLList_str[];

extern const char js_amp_entity_str[];
extern const char js_gt_entity_str[];
extern const char js_lt_entity_str[];
extern const char js_quot_entity_str[];

struct JSXMLNamespace {
    JSObject            *object;
    JSString            *prefix;
    JSString            *uri;
    JSBool              declared;       /* true if declared in its XML tag */
};

extern JSXMLNamespace *
js_NewXMLNamespace(JSContext *cx, JSString *prefix, JSString *uri,
                   JSBool declared);

extern void
js_MarkXMLNamespace(JSContext *cx, JSXMLNamespace *ns, void *arg);

extern void
js_FinalizeXMLNamespace(JSContext *cx, JSXMLNamespace *ns);

extern JSObject *
js_NewXMLNamespaceObject(JSContext *cx, JSString *prefix, JSString *uri,
                         JSBool declared);

extern JSObject *
js_GetXMLNamespaceObject(JSContext *cx, JSXMLNamespace *ns);

struct JSXMLQName {
    JSObject            *object;
    JSString            *uri;
    JSString            *prefix;
    JSString            *localName;
};

extern JSXMLQName *
js_NewXMLQName(JSContext *cx, JSString *uri, JSString *prefix,
               JSString *localName);

extern void
js_MarkXMLQName(JSContext *cx, JSXMLQName *qn, void *arg);

extern void
js_FinalizeXMLQName(JSContext *cx, JSXMLQName *qn);

extern JSObject *
js_NewXMLQNameObject(JSContext *cx, JSString *uri, JSString *prefix,
                     JSString *localName);

extern JSObject *
js_GetXMLQNameObject(JSContext *cx, JSXMLQName *qn);

extern JSObject *
js_GetAttributeNameObject(JSContext *cx, JSXMLQName *qn);

extern JSObject *
js_ConstructXMLQNameObject(JSContext *cx, jsval nsval, jsval lnval);

typedef JSBool
(* JS_DLL_CALLBACK JSIdentityOp)(const void *a, const void *b);

struct JSXMLArray {
    uint32              length;
    uint32              capacity;
    void                **vector;
    JSXMLArrayCursor    *cursors;
};

#define JSXML_PRESET_CAPACITY   JS_BIT(31)
#define JSXML_CAPACITY_MASK     JS_BITMASK(31)
#define JSXML_CAPACITY(array)   ((array)->capacity & JSXML_CAPACITY_MASK)

struct JSXMLArrayCursor {
    JSXMLArray          *array;
    uint32              index;
    JSXMLArrayCursor    *next;
    JSXMLArrayCursor    **prevp;
    void                *root;
};

/*
 * NB: don't reorder this enum without changing all array initializers that
 * depend on it in jsxml.c.
 */ 
typedef enum JSXMLClass {
    JSXML_CLASS_LIST,
    JSXML_CLASS_ELEMENT,
    JSXML_CLASS_ATTRIBUTE,
    JSXML_CLASS_PROCESSING_INSTRUCTION,
    JSXML_CLASS_TEXT,
    JSXML_CLASS_COMMENT,
    JSXML_CLASS_LIMIT
} JSXMLClass;

#define JSXML_CLASS_HAS_KIDS(class_)    ((class_) < JSXML_CLASS_ATTRIBUTE)
#define JSXML_CLASS_HAS_VALUE(class_)   ((class_) >= JSXML_CLASS_ATTRIBUTE)
#define JSXML_CLASS_HAS_NAME(class_)                                          \
    ((uintN)((class_) - JSXML_CLASS_ELEMENT) <=                               \
     (uintN)(JSXML_CLASS_PROCESSING_INSTRUCTION - JSXML_CLASS_ELEMENT))

struct JSXML {
    JSObject            *object;
    void                *domnode;       /* DOM node if mapped info item */
    JSXML               *parent;
    JSXMLQName          *name;
    uint16              xml_class;      /* discriminates u, below */
    uint16              xml_flags;      /* flags, see below */
    union {
        struct JSXMLListVar {
            JSXMLArray  kids;           /* NB: must come first */
            JSXML       *target;
            JSXMLQName  *targetprop;
        } list;
        struct JSXMLVar {
            JSXMLArray  kids;           /* NB: must come first */
            JSXMLArray  namespaces;
            JSXMLArray  attrs;
        } elem;
        JSString        *value;
    } u;

    /* Don't add anything after u -- see js_NewXML for why. */
};

/* union member shorthands */
#define xml_kids        u.list.kids
#define xml_target      u.list.target
#define xml_targetprop  u.list.targetprop
#define xml_namespaces  u.elem.namespaces
#define xml_attrs       u.elem.attrs
#define xml_value       u.value

/* xml_flags values */
#define XMLF_WHITESPACE_TEXT    0x1

/* xml_class-testing macros */
#define JSXML_HAS_KIDS(xml)     JSXML_CLASS_HAS_KIDS((xml)->xml_class)
#define JSXML_HAS_VALUE(xml)    JSXML_CLASS_HAS_VALUE((xml)->xml_class)
#define JSXML_HAS_NAME(xml)     JSXML_CLASS_HAS_NAME((xml)->xml_class)
#define JSXML_LENGTH(xml)       (JSXML_CLASS_HAS_KIDS((xml)->xml_class)       \
                                 ? (xml)->xml_kids.length                     \
                                 : 0)

extern JSXML *
js_NewXML(JSContext *cx, JSXMLClass xml_class);

extern void
js_MarkXML(JSContext *cx, JSXML *xml, void *arg);

extern void
js_FinalizeXML(JSContext *cx, JSXML *xml);

extern JSObject *
js_ParseNodeToXMLObject(JSContext *cx, JSParseNode *pn);

extern JSObject *
js_NewXMLObject(JSContext *cx, JSXMLClass xml_class);

extern JSObject *
js_GetXMLObject(JSContext *cx, JSXML *xml);

extern JS_FRIEND_DATA(JSXMLObjectOps)   js_XMLObjectOps;
extern JS_FRIEND_DATA(JSClass)          js_XMLClass;
extern JS_FRIEND_DATA(JSExtendedClass)  js_NamespaceClass;
extern JS_FRIEND_DATA(JSExtendedClass)  js_QNameClass;
extern JS_FRIEND_DATA(JSClass)          js_AttributeNameClass;
extern JS_FRIEND_DATA(JSClass)          js_AnyNameClass;

/*
 * Macros to test whether an object or a value is of type "xml" (per typeof).
 * NB: jsapi.h must be included before any call to VALUE_IS_XML.
 */
#define OBJECT_IS_XML(cx,obj)   ((obj)->map->ops == &js_XMLObjectOps.base)
#define VALUE_IS_XML(cx,v)      (!JSVAL_IS_PRIMITIVE(v) &&                    \
                                 OBJECT_IS_XML(cx, JSVAL_TO_OBJECT(v)))

extern JSObject *
js_InitNamespaceClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_InitQNameClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_InitAttributeNameClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_InitAnyNameClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_InitXMLClass(JSContext *cx, JSObject *obj);

extern JSObject *
js_InitXMLClasses(JSContext *cx, JSObject *obj);

extern JSBool
js_GetFunctionNamespace(JSContext *cx, jsval *vp);

extern JSBool
js_GetDefaultXMLNamespace(JSContext *cx, jsval *vp);

extern JSBool
js_SetDefaultXMLNamespace(JSContext *cx, jsval v);

/*
 * Return true if v is a XML QName object, or if it converts to a string that
 * contains a valid XML qualified name (one containing no :), false otherwise.
 * NB: This function is an infallible predicate, it hides exceptions.
 */
extern JSBool
js_IsXMLName(JSContext *cx, jsval v);

extern JSBool
js_ToAttributeName(JSContext *cx, jsval *vp);

extern JSString *
js_EscapeAttributeValue(JSContext *cx, JSString *str);

extern JSString *
js_AddAttributePart(JSContext *cx, JSBool isName, JSString *str,
                    JSString *str2);

extern JSString *
js_EscapeElementValue(JSContext *cx, JSString *str);

extern JSString *
js_ValueToXMLString(JSContext *cx, jsval v);

extern JSBool
js_GetAnyName(JSContext *cx, jsval *vp);

extern JSBool
js_FindXMLProperty(JSContext *cx, jsval name, JSObject **objp, jsval *namep);

extern JSBool
js_GetXMLProperty(JSContext *cx, JSObject *obj, jsval name, jsval *vp);

extern JSBool
js_SetXMLProperty(JSContext *cx, JSObject *obj, jsval name, jsval *vp);

extern JSBool
js_GetXMLDescendants(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

extern JSBool
js_DeleteXMLListElements(JSContext *cx, JSObject *listobj);

extern JSBool
js_FilterXMLList(JSContext *cx, JSObject *obj, jsbytecode *pc, jsval *vp);

extern JSObject *
js_ValueToXMLObject(JSContext *cx, jsval v);

extern JSObject *
js_ValueToXMLListObject(JSContext *cx, jsval v);

extern JSObject *
js_CloneXMLObject(JSContext *cx, JSObject *obj);

extern JSObject *
js_NewXMLSpecialObject(JSContext *cx, JSXMLClass xml_class, JSString *name,
                       JSString *value);

extern JSString *
js_MakeXMLCDATAString(JSContext *cx, JSString *str);

extern JSString *
js_MakeXMLCommentString(JSContext *cx, JSString *str);

extern JSString *
js_MakeXMLPIString(JSContext *cx, JSString *name, JSString *str);

#endif /* jsxml_h___ */
