﻿Option Explicit On
Option Strict On

Public Class DeclareNode
    Implements AstNode
    
    Public declaration As Token
    Public name As String
    Public type As String
    Public initialization As ExpressionNode

    Public Function checkTypes() As Boolean
        Return type = initialization.type
    End Function

End Class
