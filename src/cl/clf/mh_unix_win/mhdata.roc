/*
** Copyright (c) 2010 Ingres Corporation
*/

#include <compat.h>
#include <mh.h>

/*
** Constant global data for MH.
**
** History:
**       01-Dec-2009 (coomi01) b122980
**           Created.
**	11-Mar-2010 (kschendel) SIR 123275
**	    Break out powers-of-10 table so that it can be global-ized for
**	    use by CVexp10.
*/

/*
** Store the first 308 powers of 10 as double precision floats.
** (i.e. the range is 10**0 through 10**307.)
*/
GLOBALDEF f8 MHpowerTenTab [MHMAX_P10TAB] = {
  1.0    , 1.0E1, 1.0E2, 1.0E3, 1.0E4, 1.0E5, 1.0E6, 1.0E7, 1.0E8, 1.0E9, 1.0E10
, 1.0E11, 1.0E12, 1.0E13, 1.0E14, 1.0E15, 1.0E16, 1.0E17, 1.0E18, 1.0E19, 1.0E20
, 1.0E21, 1.0E22, 1.0E23, 1.0E24, 1.0E25, 1.0E26, 1.0E27, 1.0E28, 1.0E29, 1.0E30
, 1.0E31, 1.0E32, 1.0E33, 1.0E34, 1.0E35, 1.0E36, 1.0E37, 1.0E38, 1.0E39, 1.0E40
, 1.0E41, 1.0E42, 1.0E43, 1.0E44, 1.0E45, 1.0E46, 1.0E47, 1.0E48, 1.0E49, 1.0E50
, 1.0E51, 1.0E52, 1.0E53, 1.0E54, 1.0E55, 1.0E56, 1.0E57, 1.0E58, 1.0E59, 1.0E60
, 1.0E61, 1.0E62, 1.0E63, 1.0E64, 1.0E65, 1.0E66, 1.0E67, 1.0E68, 1.0E69, 1.0E70
, 1.0E71, 1.0E72, 1.0E73, 1.0E74, 1.0E75, 1.0E76, 1.0E77, 1.0E78, 1.0E79, 1.0E80
, 1.0E81, 1.0E82, 1.0E83, 1.0E84, 1.0E85, 1.0E86, 1.0E87, 1.0E88, 1.0E89, 1.0E90
, 1.0E91, 1.0E92, 1.0E93, 1.0E94, 1.0E95, 1.0E96, 1.0E97, 1.0E98, 1.0E99, 1.0E100
, 1.0E101, 1.0E102, 1.0E103, 1.0E104, 1.0E105, 1.0E106, 1.0E107, 1.0E108, 1.0E109, 1.0E110
, 1.0E111, 1.0E112, 1.0E113, 1.0E114, 1.0E115, 1.0E116, 1.0E117, 1.0E118, 1.0E119, 1.0E120
, 1.0E121, 1.0E122, 1.0E123, 1.0E124, 1.0E125, 1.0E126, 1.0E127, 1.0E128, 1.0E129, 1.0E130
, 1.0E131, 1.0E132, 1.0E133, 1.0E134, 1.0E135, 1.0E136, 1.0E137, 1.0E138, 1.0E139, 1.0E140
, 1.0E141, 1.0E142, 1.0E143, 1.0E144, 1.0E145, 1.0E146, 1.0E147, 1.0E148, 1.0E149, 1.0E150
, 1.0E151, 1.0E152, 1.0E153, 1.0E154, 1.0E155, 1.0E156, 1.0E157, 1.0E158, 1.0E159, 1.0E160
, 1.0E161, 1.0E162, 1.0E163, 1.0E164, 1.0E165, 1.0E166, 1.0E167, 1.0E168, 1.0E169, 1.0E170
, 1.0E171, 1.0E172, 1.0E173, 1.0E174, 1.0E175, 1.0E176, 1.0E177, 1.0E178, 1.0E179, 1.0E180
, 1.0E181, 1.0E182, 1.0E183, 1.0E184, 1.0E185, 1.0E186, 1.0E187, 1.0E188, 1.0E189, 1.0E190
, 1.0E191, 1.0E192, 1.0E193, 1.0E194, 1.0E195, 1.0E196, 1.0E197, 1.0E198, 1.0E199, 1.0E200
, 1.0E201, 1.0E202, 1.0E203, 1.0E204, 1.0E205, 1.0E206, 1.0E207, 1.0E208, 1.0E209, 1.0E210
, 1.0E211, 1.0E212, 1.0E213, 1.0E214, 1.0E215, 1.0E216, 1.0E217, 1.0E218, 1.0E219, 1.0E220
, 1.0E221, 1.0E222, 1.0E223, 1.0E224, 1.0E225, 1.0E226, 1.0E227, 1.0E228, 1.0E229, 1.0E230
, 1.0E231, 1.0E232, 1.0E233, 1.0E234, 1.0E235, 1.0E236, 1.0E237, 1.0E238, 1.0E239, 1.0E240
, 1.0E241, 1.0E242, 1.0E243, 1.0E244, 1.0E245, 1.0E246, 1.0E247, 1.0E248, 1.0E249, 1.0E250
, 1.0E251, 1.0E252, 1.0E253, 1.0E254, 1.0E255, 1.0E256, 1.0E257, 1.0E258, 1.0E259, 1.0E260
, 1.0E261, 1.0E262, 1.0E263, 1.0E264, 1.0E265, 1.0E266, 1.0E267, 1.0E268, 1.0E269, 1.0E270
, 1.0E271, 1.0E272, 1.0E273, 1.0E274, 1.0E275, 1.0E276, 1.0E277, 1.0E278, 1.0E279, 1.0E280
, 1.0E281, 1.0E282, 1.0E283, 1.0E284, 1.0E285, 1.0E286, 1.0E287, 1.0E288, 1.0E289, 1.0E290
, 1.0E291, 1.0E292, 1.0E293, 1.0E294, 1.0E295, 1.0E296, 1.0E297, 1.0E298, 1.0E299, 1.0E300
, 1.0E301, 1.0E302, 1.0E303, 1.0E304, 1.0E305, 1.0E306, 1.0E307};
