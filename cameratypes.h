#ifndef CAMERATYPES_H
#define CAMERATYPES_H

#include <QObject>

namespace CutePocket
{
Q_NAMESPACE

typedef union
{
    quint16 u16;
    quint8 u8[2];
} U16_U8;

typedef union
{
    qint16 s16;
    qint8 u8[2];
} S16_S8;

typedef union
{
    quint32 u32;
    quint16 u16[2];
    quint8 u8[4];
} U32_U8;

typedef union
{
    qint32 s32;
    qint8 s8[4];
} S32_S8;

typedef union
{
    qint64 s64;
    qint8  s8[8];
} S64_S8;

inline quint16 uint16at(const QByteArray &ba, int p) {
    CutePocket::U16_U8 val;
    val.u8[1] = static_cast<quint8>(ba.at(p));
    val.u8[0] = static_cast<quint8>(ba.at(p+1));
    
    return val.u16;
};

inline qint16 int16at(const QByteArray &ba, int p) {
    CutePocket::S16_S8 val;
    val.u8[1] = static_cast<quint8>(ba.at(p));
    val.u8[0] = static_cast<quint8>(ba.at(p+1));
    
    return val.s16;
};

inline quint16 uint32at(const QByteArray &ba, int p) {
    CutePocket::U32_U8 val;
    val.u8[3] = static_cast<quint8>(ba.at(p));
    val.u8[2] = static_cast<quint8>(ba.at(p+1));
    val.u8[1] = static_cast<quint8>(ba.at(p+2));
    val.u8[0] = static_cast<quint8>(ba.at(p+3));
    
    return val.u32;
};

inline qint64 int64at(const QByteArray &ba, int p) {
    CutePocket::S64_S8 val;
    val.s8[7] = static_cast<qint8>(ba.at(p));
    val.s8[6] = static_cast<qint8>(ba.at(p+1));
    val.s8[5] = static_cast<qint8>(ba.at(p+2));
    val.s8[4] = static_cast<qint8>(ba.at(p+3));
    val.s8[3] = static_cast<qint8>(ba.at(p+4));
    val.s8[2] = static_cast<qint8>(ba.at(p+5));
    val.s8[1] = static_cast<qint8>(ba.at(p+6));
    val.s8[0] = static_cast<qint8>(ba.at(p+7));
    
    return val.s64;
};

inline bool boolat(const QByteArray &ba, int p) {
    return static_cast<bool>(ba.at(p));
};


enum MediaType
{
    StillMedia = 1,
    ClipMedia = 2,
    SoundMedia = 3
};
Q_ENUM_NS(MediaType)

}
#endif // CAMERATYPES_H
