begin;

create table if not exists public.zzzombies_records (
    clave text primary key check (clave ~ '^[0-9a-f]{64}$'),
    puntos integer not null check (puntos > 0),
    oleada integer not null check (oleada >= 0),
    modo integer not null check (modo in (-1, 0, 1, 2))
);

alter table public.zzzombies_records enable row level security;
revoke all on public.zzzombies_records from public, anon, authenticated;
create index if not exists zzzombies_records_puntos on public.zzzombies_records (puntos desc);

create or replace function public.zzzombies_record(
    p_clave text, p_puntos integer, p_oleada integer, p_modo integer
) returns text
language plpgsql
security definer
set search_path = ''
as $$
declare
    personal integer;
    lider integer;
    puesto bigint := 0;
begin
    if p_clave is null or p_clave !~ '^[0-9a-f]{64}$'
        or p_puntos is null or p_puntos < 0
        or p_oleada is null or p_oleada < 0
        or p_modo is null or p_modo not in (-1, 0, 1, 2) then
        raise exception 'Récord inválido' using errcode = '22023';
    end if;

    if p_puntos > 0 then
        insert into public.zzzombies_records as anterior (clave, puntos, oleada, modo)
        values (p_clave, p_puntos, p_oleada, p_modo)
        on conflict (clave) do update
            set puntos = excluded.puntos, oleada = excluded.oleada, modo = excluded.modo
            where anterior.puntos < excluded.puntos;
    end if;

    select puntos into personal from public.zzzombies_records where clave = p_clave;
    select coalesce(max(puntos), 0) into lider from public.zzzombies_records;
    if personal is not null then
        select count(*) + 1 into puesto from public.zzzombies_records where puntos > personal;
    end if;
    return lider::text || ':' || puesto::text;
end;
$$;

revoke execute on function public.zzzombies_record(text, integer, integer, integer) from public, anon, authenticated;
grant execute on function public.zzzombies_record(text, integer, integer, integer) to anon;

commit;
