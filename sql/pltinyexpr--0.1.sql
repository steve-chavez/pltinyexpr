CREATE FUNCTION pltinyexpr_handler()
  RETURNS language_handler
  LANGUAGE c AS 'pltinyexpr';

CREATE TRUSTED LANGUAGE pltinyexpr
  HANDLER pltinyexpr_handler;
