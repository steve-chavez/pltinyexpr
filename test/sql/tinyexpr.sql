create extension pltinyexpr;

create function simple_addition() returns float8 as $$
  1 + 3
$$ language pltinyexpr;
\echo

select simple_addition();
\echo

create function more_complex() returns float8 as $$
  abs(sqrt(3^2) - 5*2)
$$ language pltinyexpr;
\echo

select more_complex();

create function failure() returns float8 as $$
  3 + trash
$$ language pltinyexpr;
\echo

select failure();
\echo

DO LANGUAGE pltinyexpr $$
  5 * 4
$$;
\echo

DO LANGUAGE pltinyexpr $$
  -)(
$$;
