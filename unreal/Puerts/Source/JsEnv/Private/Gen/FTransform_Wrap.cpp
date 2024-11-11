/*
 * Tencent is pleased to support the open source community by making Puerts available.
 * Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
 * Puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may
 * be subject to their corresponding license terms. This file is subject to the terms and conditions defined in file 'LICENSE',
 * which is part of this source code package.
 */

// gen by puerts gen tools

#include "CoreMinimal.h"
#include "Binding.hpp"
#include "UEDataBinding.hpp"

UsingUStruct(FTransform);
UsingUStruct(FVector);
UsingUStruct(FQuat);
UsingUStruct(FRotator);
UsingUStruct(FVector4);
UsingCppType(ScalarRegister);

struct AutoRegisterForFTransform
{
    AutoRegisterForFTransform()
    {
        puerts::DefineClass<FTransform>()
            .Method("DiagnosticCheckNaN_Translate", MakeFunction(&FTransform::DiagnosticCheckNaN_Translate))
            .Method("DiagnosticCheckNaN_Rotate", MakeFunction(&FTransform::DiagnosticCheckNaN_Rotate))
            .Method("DiagnosticCheckNaN_Scale3D", MakeFunction(&FTransform::DiagnosticCheckNaN_Scale3D))
            .Method("DiagnosticCheckNaN_All", MakeFunction(&FTransform::DiagnosticCheckNaN_All))
            .Method("DiagnosticCheck_IsValid", MakeFunction(&FTransform::DiagnosticCheck_IsValid))
            .Method("DebugPrint", MakeFunction(&FTransform::DebugPrint))
            .Method("ToHumanReadableString", MakeFunction(&FTransform::ToHumanReadableString))
            .Method("ToString", MakeFunction(&FTransform::ToString))
            .Method("InitFromString", MakeFunction(&FTransform::InitFromString))
            .Method("Inverse", MakeFunction(&FTransform::Inverse))
            .Method("Blend", MakeFunction(&FTransform::Blend))
            .Method("BlendWith", MakeFunction(&FTransform::BlendWith))
            .Method("op_Addition", MakeFunction(&FTransform::operator+))
            .Method("op_Multiply",
                CombineOverloads(MakeOverload(FTransform(FTransform::*)(const ScalarRegister&) const, &FTransform::operator*),
                    MakeOverload(FTransform(FTransform::*)(const FTransform&) const, &FTransform::operator*),
                    MakeOverload(FTransform(FTransform::*)(const FQuat&) const, &FTransform::operator*)))
            .Function("AnyHasNegativeScale", MakeFunction(&FTransform::AnyHasNegativeScale))
            .Method("RemoveScaling", MakeFunction(&FTransform::RemoveScaling))
            .Method("GetMaximumAxisScale", MakeFunction(&FTransform::GetMaximumAxisScale))
            .Method("GetMinimumAxisScale", MakeFunction(&FTransform::GetMinimumAxisScale))
            .Method("GetRelativeTransform", MakeFunction(&FTransform::GetRelativeTransform))
            .Method("GetRelativeTransformReverse", MakeFunction(&FTransform::GetRelativeTransformReverse))
            .Method("SetToRelativeTransform", MakeFunction(&FTransform::SetToRelativeTransform))
            .Method("TransformFVector4", MakeFunction(&FTransform::TransformFVector4))
            .Method("TransformFVector4NoScale", MakeFunction(&FTransform::TransformFVector4NoScale))
            .Method("TransformPosition", MakeFunction(&FTransform::TransformPosition))
            .Method("TransformPositionNoScale", MakeFunction(&FTransform::TransformPositionNoScale))
            .Method("InverseTransformPosition", MakeFunction(&FTransform::InverseTransformPosition))
            .Method("InverseTransformPositionNoScale", MakeFunction(&FTransform::InverseTransformPositionNoScale))
            .Method("TransformVector", MakeFunction(&FTransform::TransformVector))
            .Method("TransformVectorNoScale", MakeFunction(&FTransform::TransformVectorNoScale))
            .Method("InverseTransformVector", MakeFunction(&FTransform::InverseTransformVector))
            .Method("InverseTransformVectorNoScale", MakeFunction(&FTransform::InverseTransformVectorNoScale))
            .Method("TransformRotation", MakeFunction(&FTransform::TransformRotation))
            .Method("InverseTransformRotation", MakeFunction(&FTransform::InverseTransformRotation))
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION > 2
            .Method("ScaleTranslation",
                CombineOverloads(MakeOverload(void(FTransform::*)(const FVector&), &FTransform::ScaleTranslation),
                    MakeOverload(void(FTransform::*)(const double&), &FTransform::ScaleTranslation)))
            .Method("GetScaled", CombineOverloads(MakeOverload(FTransform(FTransform::*)(double) const, &FTransform::GetScaled),
                                     MakeOverload(FTransform(FTransform::*)(FVector) const, &FTransform::GetScaled)))
            .Function(
                "GetSafeScaleReciprocal", SelectFunction(FVector(*)(const FVector&, double), &FTransform::GetSafeScaleReciprocal))
#else
            .Method("ScaleTranslation",
                CombineOverloads(MakeOverload(void(FTransform::*)(const FVector&), &FTransform::ScaleTranslation),
                    MakeOverload(void(FTransform::*)(const float&), &FTransform::ScaleTranslation)))
            .Method("GetScaled", CombineOverloads(MakeOverload(FTransform(FTransform::*)(float) const, &FTransform::GetScaled),
                                     MakeOverload(FTransform(FTransform::*)(FVector) const, &FTransform::GetScaled)))
            .Function(
                "GetSafeScaleReciprocal", SelectFunction(FVector(*)(const FVector&, float), &FTransform::GetSafeScaleReciprocal))
#endif
            .Method("GetScaledAxis", MakeFunction(&FTransform::GetScaledAxis))
            .Method("GetUnitAxis", MakeFunction(&FTransform::GetUnitAxis))
            .Method("Mirror", MakeFunction(&FTransform::Mirror))
            .Method("GetLocation", MakeFunction(&FTransform::GetLocation))
            .Method("Rotator", MakeFunction(&FTransform::Rotator))
            .Method("GetDeterminant", MakeFunction(&FTransform::GetDeterminant))
            .Method("SetLocation", MakeFunction(&FTransform::SetLocation))
            .Method("ContainsNaN", MakeFunction(&FTransform::ContainsNaN))
            .Method("IsValid", MakeFunction(&FTransform::IsValid))
            .Function("AreRotationsEqual", MakeFunction(&FTransform::AreRotationsEqual))
            .Function("AreTranslationsEqual", MakeFunction(&FTransform::AreTranslationsEqual))
            .Function("AreScale3DsEqual", MakeFunction(&FTransform::AreScale3DsEqual))
            .Method("RotationEquals", MakeFunction(&FTransform::RotationEquals))
            .Method("TranslationEquals", MakeFunction(&FTransform::TranslationEquals))
            .Method("Scale3DEquals", MakeFunction(&FTransform::Scale3DEquals))
            .Method("Equals", MakeFunction(&FTransform::Equals))
            .Method("EqualsNoScale", MakeFunction(&FTransform::EqualsNoScale))
            .Function("Multiply", MakeFunction(&FTransform::Multiply))
            .Method("SetComponents", MakeFunction(&FTransform::SetComponents))
            .Method("SetIdentity", MakeFunction(&FTransform::SetIdentity))
            .Method("MultiplyScale3D", MakeFunction(&FTransform::MultiplyScale3D))
            .Method("SetTranslation", MakeFunction(&FTransform::SetTranslation))
            .Method("CopyTranslation", MakeFunction(&FTransform::CopyTranslation))
            .Method("ConcatenateRotation", MakeFunction(&FTransform::ConcatenateRotation))
            .Method("AddToTranslation", MakeFunction(&FTransform::AddToTranslation))
            .Function("AddTranslations", MakeFunction(&FTransform::AddTranslations))
            .Function("SubtractTranslations", MakeFunction(&FTransform::SubtractTranslations))
            .Method("SetRotation", MakeFunction(&FTransform::SetRotation))
            .Method("CopyRotation", MakeFunction(&FTransform::CopyRotation))
            .Method("SetScale3D", MakeFunction(&FTransform::SetScale3D))
            .Method("CopyScale3D", MakeFunction(&FTransform::CopyScale3D))
            .Method("SetTranslationAndScale3D", MakeFunction(&FTransform::SetTranslationAndScale3D))
            .Method("Accumulate",
                CombineOverloads(MakeOverload(void(FTransform::*)(const FTransform&), &FTransform::Accumulate),
                    MakeOverload(void(FTransform::*)(const FTransform&, const ScalarRegister&), &FTransform::Accumulate)))
            .Method("NormalizeRotation", MakeFunction(&FTransform::NormalizeRotation))
            .Method("IsRotationNormalized", MakeFunction(&FTransform::IsRotationNormalized))
            .Method("GetRotation", MakeFunction(&FTransform::GetRotation))
            .Method("GetTranslation", MakeFunction(&FTransform::GetTranslation))
            .Method("GetScale3D", MakeFunction(&FTransform::GetScale3D))
            .Method("CopyRotationPart", MakeFunction(&FTransform::CopyRotationPart))
            .Method("CopyTranslationAndScale3D", MakeFunction(&FTransform::CopyTranslationAndScale3D))
            .Register();
    }
};

AutoRegisterForFTransform _AutoRegisterForFTransform_;