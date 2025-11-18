let max_output = 200

(* Turn input into a list of words. Keep punctuation attached to words. *)
let input =
  Str.split (Str.regexp "[ \t\r\n]+") (In_channel.input_all In_channel.stdin)

module Prefix = struct
  type t = string * string

  let compare (s1, s2) (s3, s4) = String.compare (s1 ^ s2) (s3 ^ s4)
end

module PrefixMap = Map.Make (Prefix)

(* Collisions will be stored in Dynarrays *)
type suffix_bucket = string option Dynarray.t

let build_state_map =
  let rec build_state_map' (map : suffix_bucket PrefixMap.t)
      (input : string list) =
    match input with
    | w1 :: w2 :: t -> begin
        let prefix = (w1, w2) in
        let w3 = if t = [] then None else Some (List.hd t) in
        let bucket =
          match PrefixMap.find_opt prefix map with
          | None -> Dynarray.make 1 w3
          (* Intentionally allow duplicates in a bucket. This makes more common
          suffixes naturally more common in the generated output. *)
          | Some arr ->
              Dynarray.add_last arr w3;
              arr
        in
        let map' = PrefixMap.add prefix bucket map in
        build_state_map' map' (List.tl input)
      end
    | _ -> map
  in
  build_state_map' PrefixMap.empty

let () = Random.self_init ()

let make_prefix = function
  | w1 :: w2 :: _ -> (w1, w2)
  | _ -> raise (Invalid_argument "Input shorter than prefix length")

let generate n =
  (* Start off generation with the same start as the input text *)
  let start_prefix = make_prefix input in
  let () = Printf.printf "%s %s " (fst start_prefix) (snd start_prefix) in
  let rec generate' n state_map prefix =
    if n = 0 then print_newline ()
    else
      match PrefixMap.find_opt prefix state_map with
      | None -> print_newline ()
      | Some bucket -> begin
          let idx = Random.int (Dynarray.length bucket) in
          match Dynarray.get bucket idx with
          | None -> print_newline ()
          | Some str ->
              Printf.printf "%s " str;
              generate' (n - 1) state_map (snd prefix, str)
        end
  in
  generate' n (build_state_map input) start_prefix

let () = generate max_output
