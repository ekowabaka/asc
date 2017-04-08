﻿
Option Explicit On
Option Strict On

Imports System

Public MustInherit Class AstNodeParser
    Private Shared _parser As Parser

    Public Shared ReadOnly Property parser() As Parser
        Get
            Return _parser
        End Get
    End Property

    Public Shared Function init() As Parser
        ExpressionParser.init()
        Lexer.initializeRules()
        _parser = New Parser()
        Return _parser
    End Function

    Public MustOverride Function parse() As AstNode
End Class

