// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Reflection.Metadata;
using System.Reflection.Metadata.Ecma335;
using System.Text;

using Internal.CorConstants;
using Internal.ReadyToRunConstants;

namespace ILCompiler.Reflection.ReadyToRun
{
    public interface IR2RSignatureTypeProvider<TType, TMethod, TGenericContext> : ISignatureTypeProvider<TType, TGenericContext>
    {
        TType GetCanonType();
        TMethod GetMethodFromMethodDef(MetadataReader reader, MethodDefinitionHandle handle, TType owningTypeOverride);
        TMethod GetMethodFromMemberRef(MetadataReader reader, MemberReferenceHandle handle, TType owningTypeOverride);
        TMethod GetInstantiatedMethod(TMethod uninstantiatedMethod, ImmutableArray<TType> instantiation);
        TMethod GetConstrainedMethod(TMethod method, TType constraint);
        TMethod GetMethodWithFlags(ReadyToRunMethodSigFlags flags, TMethod method);
    }

    /// <summary>
    /// Helper class used as state machine for decoding a single signature.
    /// </summary>
    public class R2RSignatureDecoder<TSignature, TType, TMethod, TGenericContext>
    {
        /// <summary>
        /// ECMA reader is used to access the embedded MSIL metadata blob in the R2R file.
        /// </summary>
        protected readonly MetadataReader _metadataReader;

        /// <summary>
        /// Outer ECMA reader is used as the default context for generic parameters.
        /// </summary>
        private readonly MetadataReader _outerReader;

        /// <summary>
        /// ECMA reader representing the reference module of the signature being decoded.
        /// </summary>
        protected readonly ReadyToRunReader _contextReader;

        /// <summary>
        /// Byte array representing the R2R PE file read from disk.
        /// </summary>
        protected readonly byte[] _image;

        /// <summary>
        /// Offset within the image file.
        /// </summary>
        private int _offset;

        /// <summary>
        /// Offset within the image file when the object is constructed.
        /// </summary>
        private readonly int _originalOffset;

        /// <summary>
        /// Query signature parser for the current offset.
        /// </summary>
        public int Offset => _offset;

        private IR2RSignatureTypeProvider<TType, TMethod, TGenericContext> _provider;

        protected void UpdateOffset(int offset)
        {
            _offset = offset;
        }

        public TGenericContext Context { get; }

        /// <summary>
        /// Construct the signature decoder by storing the image byte array and offset within the array.
        /// </summary>
        /// <param name="r2rReader">R2RReader object representing the PE file containing the ECMA metadata</param>
        /// <param name="offset">Signature offset within the PE file byte array</param>
        public R2RSignatureDecoder(IR2RSignatureTypeProvider<TType, TMethod, TGenericContext> provider, TGenericContext context, MetadataReader metadataReader, ReadyToRunReader r2rReader, int offset, bool skipOverrideMetadataReader = false)
        {
            Context = context;
            _provider = provider;
            _image = r2rReader.Image;
            _originalOffset = _offset = offset;
            _contextReader = r2rReader;
            MetadataReader moduleOverrideMetadataReader = null;
            if (!skipOverrideMetadataReader)
                moduleOverrideMetadataReader = TryGetModuleOverrideMetadataReader();
            _metadataReader = moduleOverrideMetadataReader ?? metadataReader;
            _outerReader = moduleOverrideMetadataReader ?? metadataReader;
            Reset();
        }

        /// <summary>
        /// Construct the signature decoder by storing the image byte array and offset within the array.
        /// </summary>
        /// <param name="options">Dump options and paths</param>
        /// <param name="metadataReader">Metadata reader for the R2R image</param>
        /// <param name="signature">Signature to parse</param>
        /// <param name="offset">Signature offset within the signature byte array</param>
        /// <param name="outerReader">Metadata reader representing the outer signature context</param>
        /// <param name="contextReader">Top-level signature context reader</param>
        public R2RSignatureDecoder(IR2RSignatureTypeProvider<TType, TMethod, TGenericContext> provider, TGenericContext context, MetadataReader metadataReader, byte[] signature, int offset, MetadataReader outerReader, ReadyToRunReader contextReader, bool skipOverrideMetadataReader = false)
        {
            Context = context;
            _provider = provider;
            _image = signature;
            _originalOffset = _offset = offset;
            _contextReader = contextReader;
            MetadataReader moduleOverrideMetadataReader = null;
            if (!skipOverrideMetadataReader)
                moduleOverrideMetadataReader = TryGetModuleOverrideMetadataReader();
            _metadataReader = moduleOverrideMetadataReader ?? metadataReader;
            _outerReader = moduleOverrideMetadataReader ?? outerReader;
            Reset();
        }

        private MetadataReader TryGetModuleOverrideMetadataReader()
        {
            bool moduleOverride = (ReadByte() & (byte)ReadyToRunFixupKind.ModuleOverride) != 0;
            // Check first byte for a module override being encoded
            if (moduleOverride)
            {
                int moduleIndex = (int)ReadUInt();
                IAssemblyMetadata refAsmEcmaReader = _contextReader.OpenReferenceAssembly(moduleIndex);
                return refAsmEcmaReader.MetadataReader;
            }

            return null;
        }

        /// <summary>
        /// Reset the offset back to the point where the decoder is constructed to allow re-decoding the same signature.
        /// </summary>
        internal void Reset()
        {
            this._offset = _originalOffset;
        }

        /// <summary>
        /// Read a single byte from the signature stream and advances the current offset.
        /// </summary>
        public byte ReadByte()
        {
            return _image[_offset++];
        }

        /// <summary>
        /// Read a single unsigned 32-bit in from the signature stream. Adapted from CorSigUncompressData,
        /// <a href="">https://github.com/dotnet/coreclr/blob/master/src/inc/cor.h</a>.
        /// </summary>
        /// <param name="data"></param>
        public uint ReadUInt()
        {
            // Handle smallest data inline.
            byte firstByte = ReadByte();
            if ((firstByte & 0x80) == 0x00) // 0??? ????
                return firstByte;

            uint res;
            // Medium.
            if ((firstByte & 0xC0) == 0x80)  // 10?? ????
            {
                res = ((uint)(firstByte & 0x3f) << 8);
                res |= ReadByte();
            }
            else // 110? ????
            {
                res = (uint)(firstByte & 0x1f) << 24;
                res |= (uint)ReadByte() << 16;
                res |= (uint)ReadByte() << 8;
                res |= (uint)ReadByte();
            }
            return res;
        }

        /// <summary>
        /// Read a signed integer from the signature stream. Signed integer is basically encoded
        /// as an unsigned integer after converting it to the unsigned number 2 * abs(x) + (x &gt;= 0 ? 0 : 1).
        /// Adapted from CorSigUncompressSignedInt, <a href="">https://github.com/dotnet/coreclr/blob/master/src/inc/cor.h</a>.
        /// </summary>
        public int ReadInt()
        {
            uint rawData = ReadUInt();
            int data = (int)(rawData >> 1);
            return ((rawData & 1) == 0 ? +data : -data);
        }

        /// <summary>
        /// Read an encoded token from the stream. This encoding left-shifts the token RID twice and
        /// fills in the two least-important bits with token type (typeDef, typeRef, typeSpec, baseType).
        /// </summary>
        public uint ReadToken()
        {
            uint encodedToken = ReadUInt();
            uint rid = encodedToken >> 2;
            CorTokenType type;
            switch (encodedToken & 3)
            {
                case 0:
                    type = CorTokenType.mdtTypeDef;
                    break;

                case 1:
                    type = CorTokenType.mdtTypeRef;
                    break;

                case 2:
                    type = CorTokenType.mdtTypeSpec;
                    break;

                case 3:
                    type = CorTokenType.mdtBaseType;
                    break;

                default:
                    // This should never happen
                    throw new NotImplementedException();
            }
            return (uint)type | rid;
        }

        /// <summary>
        /// Read a single element type from the signature stream. Adapted from CorSigUncompressElementType,
        /// <a href="">https://github.com/dotnet/coreclr/blob/master/src/inc/cor.h</a>.
        /// </summary>
        /// <returns></returns>
        public CorElementType ReadElementType()
        {
            return (CorElementType)(ReadByte() & 0x7F);
        }

        public CorElementType PeekElementType()
        {
            return (CorElementType)(_image[_offset] & 0x7F);
        }

        /// <summary>
        /// Decode a type from the signature stream.
        /// </summary>
        /// <param name="builder"></param>
        public TType ParseType()
        {
            CorElementType corElemType = ReadElementType();
            switch (corElemType)
            {
                case CorElementType.ELEMENT_TYPE_VOID:
                case CorElementType.ELEMENT_TYPE_BOOLEAN:
                case CorElementType.ELEMENT_TYPE_CHAR:
                case CorElementType.ELEMENT_TYPE_I1:
                case CorElementType.ELEMENT_TYPE_U1:
                case CorElementType.ELEMENT_TYPE_I2:
                case CorElementType.ELEMENT_TYPE_U2:
                case CorElementType.ELEMENT_TYPE_I4:
                case CorElementType.ELEMENT_TYPE_I8:
                case CorElementType.ELEMENT_TYPE_U4:
                case CorElementType.ELEMENT_TYPE_U8:
                case CorElementType.ELEMENT_TYPE_R4:
                case CorElementType.ELEMENT_TYPE_R8:
                case CorElementType.ELEMENT_TYPE_STRING:
                case CorElementType.ELEMENT_TYPE_OBJECT:
                case CorElementType.ELEMENT_TYPE_I:
                case CorElementType.ELEMENT_TYPE_U:
                case CorElementType.ELEMENT_TYPE_TYPEDBYREF:
                    return _provider.GetPrimitiveType((PrimitiveTypeCode)corElemType);

                case CorElementType.ELEMENT_TYPE_PTR:
                    return _provider.GetPointerType(ParseType());

                case CorElementType.ELEMENT_TYPE_BYREF:
                    return _provider.GetByReferenceType(ParseType());

                case CorElementType.ELEMENT_TYPE_VALUETYPE:
                case CorElementType.ELEMENT_TYPE_CLASS:
                    return ParseTypeDefOrRef(corElemType);

                case CorElementType.ELEMENT_TYPE_VAR:
                    {
                        uint varIndex = ReadUInt();
                        return _provider.GetGenericTypeParameter(Context, (int)varIndex);
                    }

                case CorElementType.ELEMENT_TYPE_ARRAY:
                    {
                        TType elementType = ParseType();
                        uint rank = ReadUInt();
                        if (rank == 0)
                            return _provider.GetSZArrayType(elementType);

                        uint sizeCount = ReadUInt(); // number of sizes
                        uint[] sizes = new uint[sizeCount];
                        for (uint sizeIndex = 0; sizeIndex < sizeCount; sizeIndex++)
                        {
                            sizes[sizeIndex] = ReadUInt();
                        }
                        uint lowerBoundCount = ReadUInt(); // number of lower bounds
                        int[] lowerBounds = new int[lowerBoundCount];
                        for (uint lowerBoundIndex = 0; lowerBoundIndex < lowerBoundCount; lowerBoundIndex++)
                        {
                            lowerBounds[lowerBoundIndex] = ReadInt();
                        }
                        ArrayShape arrayShape = new ArrayShape((int)rank, ((int[])(object)sizes).ToImmutableArray(), lowerBounds.ToImmutableArray());
                        return _provider.GetArrayType(elementType, arrayShape);
                    }

                case CorElementType.ELEMENT_TYPE_GENERICINST:
                    {
                        TType genericType = ParseType();
                        uint typeArgCount = ReadUInt();
                        var outerDecoder = new R2RSignatureDecoder<TSignature, TType, TMethod, TGenericContext>(_provider, Context, _outerReader, _image, _offset, _outerReader, _contextReader);
                        List<TType> parsedTypes = new List<TType>();
                        for (uint paramIndex = 0; paramIndex < typeArgCount; paramIndex++)
                        {
                            parsedTypes.Add(outerDecoder.ParseType());
                        }
                        _offset = outerDecoder.Offset;
                        return _provider.GetGenericInstantiation(genericType, parsedTypes.ToImmutableArray());
                    }

                case CorElementType.ELEMENT_TYPE_FNPTR:
                    var sigHeader = new SignatureHeader(ReadByte());
                    int genericParamCount = 0;
                    if (sigHeader.IsGeneric)
                    {
                        genericParamCount = (int)ReadUInt();
                    }
                    int paramCount = (int)ReadUInt();
                    TType returnType = ParseType();
                    TType[] paramTypes = new TType[paramCount];
                    int requiredParamCount = -1;
                    for (int i = 0; i < paramCount; i++)
                    {
                        while (PeekElementType() == CorElementType.ELEMENT_TYPE_SENTINEL)
                        {
                            requiredParamCount = i;
                            ReadElementType(); // Skip over sentinel
                        }
                        paramTypes[i] = ParseType();
                    }
                    if (requiredParamCount == -1)
                        requiredParamCount = paramCount;

                    MethodSignature<TType> methodSig = new MethodSignature<TType>(sigHeader, returnType, requiredParamCount, genericParamCount, paramTypes.ToImmutableArray());
                    return _provider.GetFunctionPointerType(methodSig);

                case CorElementType.ELEMENT_TYPE_SZARRAY:
                    return _provider.GetSZArrayType(ParseType());

                case CorElementType.ELEMENT_TYPE_MVAR:
                    {
                        uint varIndex = ReadUInt();
                        return _provider.GetGenericMethodParameter(Context, (int)varIndex);
                    }

                case CorElementType.ELEMENT_TYPE_CMOD_REQD:
                    return _provider.GetModifiedType(ParseTypeDefOrRefOrSpec(corElemType), ParseType(), true);

                case CorElementType.ELEMENT_TYPE_CMOD_OPT:
                    return _provider.GetModifiedType(ParseTypeDefOrRefOrSpec(corElemType), ParseType(), false);

                case CorElementType.ELEMENT_TYPE_HANDLE:
                    throw new BadImageFormatException("handle");

                case CorElementType.ELEMENT_TYPE_SENTINEL:
                    throw new BadImageFormatException("sentinel");

                case CorElementType.ELEMENT_TYPE_PINNED:
                    return _provider.GetPinnedType(ParseType());

                case CorElementType.ELEMENT_TYPE_VAR_ZAPSIG:
                    throw new BadImageFormatException("var_zapsig");

                case CorElementType.ELEMENT_TYPE_NATIVE_VALUETYPE_ZAPSIG:
                    throw new BadImageFormatException("native_valuetype_zapsig");

                case CorElementType.ELEMENT_TYPE_CANON_ZAPSIG:
                    return _provider.GetCanonType();

                case CorElementType.ELEMENT_TYPE_MODULE_ZAPSIG:
                    {
                        int moduleIndex = (int)ReadUInt();
                        IAssemblyMetadata refAsmReader = _contextReader.OpenReferenceAssembly(moduleIndex);
                        var refAsmDecoder = new R2RSignatureDecoder<TSignature, TType, TMethod, TGenericContext>(_provider, Context, refAsmReader.MetadataReader, _image, _offset, _outerReader, _contextReader);
                        var result = refAsmDecoder.ParseType();
                        _offset = refAsmDecoder.Offset;
                        return result;
                    }

                default:
                    throw new NotImplementedException();
            }
        }


        private TType ParseTypeDefOrRef(CorElementType corElemType)
        {
            uint token = ReadToken();
            var handle = MetadataTokens.Handle((int)token);
            switch (handle.Kind)
            {
                case HandleKind.TypeDefinition:
                    return _provider.GetTypeFromDefinition(_metadataReader, (TypeDefinitionHandle)handle, (byte)corElemType);
                case HandleKind.TypeReference:
                    return _provider.GetTypeFromReference(_metadataReader, (TypeReferenceHandle)handle, (byte)corElemType);
                default:
                    throw new BadImageFormatException();
            }
        }

        private TType ParseTypeDefOrRefOrSpec(CorElementType corElemType)
        {
            uint token = ReadToken();
            var handle = MetadataTokens.Handle((int)token);
            switch (handle.Kind)
            {
                case HandleKind.TypeDefinition:
                    return _provider.GetTypeFromDefinition(_metadataReader, (TypeDefinitionHandle)handle, (byte)corElemType);
                case HandleKind.TypeReference:
                    return _provider.GetTypeFromReference(_metadataReader, (TypeReferenceHandle)handle, (byte)corElemType);
                case HandleKind.TypeSpecification:
                    return _provider.GetTypeFromSpecification(_metadataReader, Context, (TypeSpecificationHandle)handle, (byte)corElemType);
                default:
                    throw new BadImageFormatException();
            }
        }

        public TMethod ParseMethod()
        {
            uint methodFlags = ReadUInt();

            if ((methodFlags & (uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_UpdateContext) != 0)
            {
                int moduleIndex = (int)ReadUInt();
                IAssemblyMetadata refAsmReader = _contextReader.OpenReferenceAssembly(moduleIndex);
                var refAsmDecoder = new R2RSignatureDecoder<TSignature, TType, TMethod, TGenericContext>(_provider, Context, refAsmReader.MetadataReader, _image, _offset, _outerReader, _contextReader, skipOverrideMetadataReader: true);
                var result = refAsmDecoder.ParseMethodWithMethodFlags(methodFlags);
                _offset = refAsmDecoder.Offset;
                return result;
            }
            else
            {
                return ParseMethodWithMethodFlags(methodFlags);
            }
        }


        private TMethod ParseMethodWithMethodFlags(uint methodFlags)
        {
            TType owningTypeOverride = default(TType);
            if ((methodFlags & (uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_OwnerType) != 0)
            {
                owningTypeOverride = ParseType();
                methodFlags &= ~(uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_OwnerType;
            }

            if ((methodFlags & (uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_SlotInsteadOfToken) != 0)
            {
                throw new NotImplementedException();
            }

            TMethod result;
            if ((methodFlags & (uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_MemberRefToken) != 0)
            {
                methodFlags &= ~(uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_MemberRefToken;
                result = ParseMethodRefToken(owningTypeOverride: owningTypeOverride);
            }
            else
            {
                result = ParseMethodDefToken(owningTypeOverride: owningTypeOverride);
            }

            if ((methodFlags & (uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_MethodInstantiation) != 0)
            {
                methodFlags &= ~(uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_MethodInstantiation;
                uint typeArgCount = ReadUInt();
                TType[] instantiationArgs = new TType[typeArgCount];
                for (int typeArgIndex = 0; typeArgIndex < typeArgCount; typeArgIndex++)
                {
                    instantiationArgs[typeArgIndex] = ParseType();
                }
                result = _provider.GetInstantiatedMethod(result, instantiationArgs.ToImmutableArray());
            }

            if ((methodFlags & (uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_Constrained) != 0)
            {
                methodFlags &= ~(uint)ReadyToRunMethodSigFlags.READYTORUN_METHOD_SIG_Constrained;
                result = _provider.GetConstrainedMethod(result, ParseType());
            }

            // Any other flags should just be directly recorded
            if (methodFlags != 0)
                result = _provider.GetMethodWithFlags((ReadyToRunMethodSigFlags)methodFlags, result);

            return result;
        }

        /// <summary>
        /// Read a methodDef token from the signature and output the corresponding object to the builder.
        /// </summary>
        /// <param name="builder">Output string builder</param>
        private TMethod ParseMethodDefToken(TType owningTypeOverride)
        {
            uint rid = ReadUInt();
            return _provider.GetMethodFromMethodDef(_metadataReader, MetadataTokens.MethodDefinitionHandle((int)rid), owningTypeOverride);
        }

        /// <summary>
        /// Read a memberRef token from the signature and output the corresponding object to the builder.
        /// </summary>
        /// <param name="builder">Output string builder</param>
        /// <param name="owningTypeOverride">Explicit owning type override</param>
        private TMethod ParseMethodRefToken(TType owningTypeOverride)
        {
            uint rid = ReadUInt();
            return _provider.GetMethodFromMemberRef(_metadataReader, MetadataTokens.MemberReferenceHandle((int)rid), owningTypeOverride);
        }

        /// <summary>
        /// Parse the signature
        /// </summary>
        public TSignature ParseSignature()
        {
            uint fixupType = ReadByte();
            // EmitInlineSignatureBinaryBytes(builder, 1);
            bool moduleOverride = (fixupType & (byte)ReadyToRunFixupKind.ModuleOverride) != 0;
            // SignatureDecoder moduleDecoder = this;

            // Check first byte for a module override being encoded. The metadata reader for the module
            // override is configured in the R2RSignatureDecoder constructor.
            if (moduleOverride)
            {
                fixupType &= ~(uint)ReadyToRunFixupKind.ModuleOverride;
                // ReadUIntAndEmitInlineSignatureBinary(builder);
            }

            return ParseSignature((ReadyToRunFixupKind)fixupType);;
        }

        private TSignature ParseSignature(ReadyToRunFixupKind fixupType)
        {
            // TODO: Copy the implementation over
            return default(TSignature);
        }
    }
}